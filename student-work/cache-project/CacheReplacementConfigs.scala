package chipyard

import org.chipsalliance.cde.config._
import freechips.rocketchip.subsystem._
import freechips.rocketchip.rocket._
import freechips.rocketchip.tile._

/*
 * Random replacement baseline
 *
 * 64 sets × 4 ways × 64 bytes = 16 KiB D-cache
 */
class Random4WayConfig extends Config(
  new Config((site, here, up) => {
    case TilesLocated(InSubsystem) =>
      up(TilesLocated(InSubsystem), site) map {
        case tp: RocketTileAttachParams =>
          tp.copy(tileParams = tp.tileParams.copy(
            dcache = tp.tileParams.dcache.map(_.copy(
              nWays = 4,
              replacementPolicy = "random"
            ))
          ))
      }
  }) ++
  new BaselineConfig
)

/*
 * LRU replacement
 *
 * Same cache geometry as Random4WayConfig.
 * The only experimental variable is the replacement policy.
 */
class LRU4WayConfig extends Config(
  new Config((site, here, up) => {
    case TilesLocated(InSubsystem) =>
      up(TilesLocated(InSubsystem), site) map {
        case tp: RocketTileAttachParams =>
          tp.copy(tileParams = tp.tileParams.copy(
            dcache = tp.tileParams.dcache.map(_.copy(
              nWays = 4,
              replacementPolicy = "lru"
            ))
          ))
      }
  }) ++
  new BaselineConfig
)