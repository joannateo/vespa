// Copyright 2017 Yahoo Holdings. Licensed under the terms of the Apache 2.0 license. See LICENSE in the project root.
#include "distributormetricsset.h"
#include <vespa/metrics/loadmetric.hpp>
#include <vespa/metrics/summetric.hpp>

namespace storage {

using metrics::MetricSet;

DistributorMetricSet::DistributorMetricSet(const metrics::LoadTypeSet& lt)
    : MetricSet("distributor", "distributor", ""),
      puts(lt, PersistenceOperationMetricSet("puts"), this),
      updates(lt, PersistenceOperationMetricSet("updates"), this),
      update_puts(lt, PersistenceOperationMetricSet("update_puts"), this),
      update_gets(lt, PersistenceOperationMetricSet("update_gets"), this),
      removes(lt, PersistenceOperationMetricSet("removes"), this),
      removelocations(lt, PersistenceOperationMetricSet("removelocations"), this),
      gets(lt, PersistenceOperationMetricSet("gets"), this),
      stats(lt, PersistenceOperationMetricSet("stats"), this),
      multioperations(lt, PersistenceOperationMetricSet("multioperations"), this),
      visits(lt, VisitorMetricSet(), this),
      stateTransitionTime("state_transition_time", "",
              "Time it takes to complete a cluster state transition. If a "
              "state transition is preempted before completing, its elapsed "
              "time is counted as part of the total time spent for the final, "
              "completed state transition", this),
      recoveryModeTime("recoverymodeschedulingtime", "",
              "Time spent scheduling operations in recovery mode "
              "after receiving new cluster state", this),
      docsStored("docsstored", "logdefault yamasdefault",
              "Number of documents stored in all buckets controlled by "
              "this distributor", this),
      bytesStored("bytesstored", "logdefault yamasdefault",
              "Number of bytes stored in all buckets controlled by "
              "this distributor", this)
{
    docsStored.logOnlyIfSet();
    bytesStored.logOnlyIfSet();
}

DistributorMetricSet::~DistributorMetricSet() { }

} // storage

template class metrics::LoadMetric<storage::PersistenceOperationMetricSet>;
template class metrics::SumMetric<storage::PersistenceOperationMetricSet>;
