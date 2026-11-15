package chipyard

import org.chipsalliance.cde.config._
import freechips.rocketchip.subsystem._
import freechips.rocketchip.rocket._
import freechips.rocketchip.tile._

class WithSRRIPDCache(
  sets: Int,
  ways: Int
) extends Config(
  new Config((site, here, up) => {
    case TilesLocated(InSubsystem) =>
      up(TilesLocated(InSubsystem), site) map {
        case tp: RocketTileAttachParams =>
          tp.copy(tileParams = tp.tileParams.copy(
            dcache = tp.tileParams.dcache.map(_.copy(
              nSets = sets,
              nWays = ways,
              replacementPolicy = "srrip"
            ))
          ))
      }
  })
)

class SRRIP2WayConfig extends Config(
  new WithSRRIPDCache(64, 2) ++
  new BaselineConfig
)

class SRRIP4WayConfig extends Config(
  new WithSRRIPDCache(32, 4) ++
  new BaselineConfig
)

class SRRIP8WayConfig extends Config(
  new WithSRRIPDCache(16, 8) ++
  new BaselineConfig
)

class SRRIP16WayConfig extends Config(
  new WithSRRIPDCache(8, 16) ++
  new BaselineConfig
)