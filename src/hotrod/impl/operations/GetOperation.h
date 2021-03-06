#ifndef ISPN_HOTROD_OPERATIONS_GETOPERATION_H
#define ISPN_HOTROD_OPERATIONS_GETOPERATION_H



#include "infinispan/hotrod/types.h"
#include "infinispan/hotrod/ScopedBuffer.h"
#include "infinispan/hotrod/RemoteCacheBase.h"
#include "hotrod/impl/operations/AbstractKeyOperation.h"

namespace infinispan {
namespace hotrod {
namespace operations {

class GetOperation : public AbstractKeyOperation<hrbytes>
{
  protected:
    hrbytes executeOperation(
        infinispan::hotrod::transport::Transport& transport);

  private:
    GetOperation(
        const infinispan::hotrod::protocol::Codec& codec,
        infinispan::hotrod::transport::TransportFactory* transportFactory,
        const hrbytes& key, const hrbytes& cacheName,
        uint32_t topologyId, uint32_t flags);

  friend class OperationsFactory;
};

}}} // namespace infinispan::hotrod::operations

#endif  // ISPN_HOTROD_OPERATIONS_GETOPERATION_H
