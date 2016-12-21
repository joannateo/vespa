// Copyright 2016 Yahoo Inc. Licensed under the terms of the Apache 2.0 license. See LICENSE in the project root.

#include "memoryconfigstore.h"

#include <vespa/log/log.h>
LOG_SETUP(".proton.server.memoryconfigstore");

namespace proton {

MemoryConfigStore::MemoryConfigStore() : _maps(new ConfigMaps) {}
MemoryConfigStore::MemoryConfigStore(ConfigMaps::SP maps) : _maps(maps) {}
MemoryConfigStore::~MemoryConfigStore() { }

ConfigStore::SerialNum
MemoryConfigStore::getBestSerialNum() const {
    return _maps->_valid.empty() ? 0 : *_maps->_valid.rbegin();
}
ConfigStore::SerialNum
MemoryConfigStore::getOldestSerialNum() const {
    return _maps->_valid.empty() ? 0 : *_maps->_valid.begin();
}
bool
MemoryConfigStore::hasValidSerial(SerialNum serial) const {
    return _maps->_valid.find(serial) != _maps->_valid.end();
}
ConfigStore::SerialNum
MemoryConfigStore::getPrevValidSerial(SerialNum serial) const {
    if (_maps->_valid.empty() ||
        *_maps->_valid.begin() >= serial) {
        return 0;
    }
    return *(--(_maps->_valid.lower_bound(serial)));
}
void
MemoryConfigStore::saveConfig(const DocumentDBConfig &config,
                              const Schema &history,
                              SerialNum serial)
{
    _maps->configs[serial].reset(new DocumentDBConfig(config));
    _maps->histories[serial].reset(new Schema(history));
    _maps->_valid.insert(serial);
}
void
MemoryConfigStore::loadConfig(const DocumentDBConfig &, SerialNum serial,
                              DocumentDBConfig::SP &loaded_config,
                              Schema::SP &history_schema)
{
    assert(hasValidSerial(serial));
    loaded_config = _maps->configs[serial];
    history_schema = _maps->histories[serial];
}
void
MemoryConfigStore::removeInvalid()
{
    // Note: Depends on C++11 semantics for erase
    for (auto it = _maps->configs.begin(); it != _maps->configs.end();) {
        if (!hasValidSerial(it->first)) {
            it = _maps->configs.erase(it);
            continue;
        }
        ++it;
    }
    for (auto it = _maps->histories.begin();
         it != _maps->histories.end();) {
        if (!hasValidSerial(it->first)) {
            it = _maps->histories.erase(it);
            continue;
        }
        ++it;
    }
}
void
MemoryConfigStore::prune(SerialNum serial) {
    _maps->configs.erase(_maps->configs.begin(),
                         _maps->configs.upper_bound(serial));
    _maps->histories.erase(_maps->histories.begin(),
                           _maps->histories.upper_bound(serial));
    _maps->_valid.erase(_maps->_valid.begin(),
                        _maps->_valid.upper_bound(serial));
}
void
MemoryConfigStore::saveWipeHistoryConfig(SerialNum serial,
                                   fastos::TimeStamp wipeTimeLimit)
{
    if (hasValidSerial(serial)) {
        return;
    }
    SerialNum prev = getPrevValidSerial(serial);
    Schema::SP schema(new Schema);
    if (wipeTimeLimit != 0) {
        Schema::SP oldHistorySchema(_maps->histories[prev]);
        Schema::UP wipeSchema;
        wipeSchema = oldHistorySchema->getOldFields(wipeTimeLimit);
        schema.reset(Schema::set_difference(*oldHistorySchema, *wipeSchema).release());
        
    }
    _maps->histories[serial] = schema;
    _maps->configs[serial] = _maps->configs[prev];
    _maps->_valid.insert(serial);
}
void
MemoryConfigStore::serializeConfig(SerialNum, vespalib::nbostream &) {
    LOG(info, "Serialization of config not implemented.");
}
void
MemoryConfigStore::deserializeConfig(SerialNum, vespalib::nbostream &) {
    assert(!"Not implemented");
}
void
MemoryConfigStore::setProtonConfig(const ProtonConfigSP &) { }

MemoryConfigStores::MemoryConfigStores() { }
MemoryConfigStores::~MemoryConfigStores() { }

ConfigStore::UP
MemoryConfigStores::getConfigStore(const std::string &type) {
    if (!_config_maps[type].get()) {
        _config_maps[type].reset(new ConfigMaps);
    }
    return ConfigStore::UP(new MemoryConfigStore(_config_maps[type]));
}

}  // namespace proton
