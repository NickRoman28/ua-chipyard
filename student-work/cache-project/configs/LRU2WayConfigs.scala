package chipyard

import org.chipsalliance.cde.config._
import freechips.rocketchip.subsystem._
import freechips.rocketchip.rocket._
import freechips.rocketchip.tile._

/*
 * 8 KiB, 2-way D-cache
 * Random replacement
 *
 * 64 sets × 2 ways × 64 bytes = 8192 bytes
 */
class Random2WayConfig extends Config(
  new Config((site, here, up) => {
    case TilesLocated(InSubsystem) =>
      up(TilesLocated(InSubsystem), site) map {
        case tp: RocketTileAttachParams =>
          tp.copy(tileParams = tp.tileParams.copy(
            dcache = tp.tileParams.dcache.map(_.copy(
              nSets = 64,
              nWays = 2,
              replacementPolicy = "random"
            ))
          ))
      }
  }) ++
  new BaselineConfig
)

/*
 * 8 KiB, 2-way D-cache
 * True LRU replacement
 *
 * Same cache geometry as Random2WayConfig.
 */
class LRU2WayConfig extends Config(
  new Config((site, here, up) => {
    case TilesLocated(InSubsystem) =>
      up(TilesLocated(InSubsystem), site) map {
        case tp: RocketTileAttachParams =>
          tp.copy(tileParams = tp.tileParams.copy(
            dcache = tp.tileParams.dcache.map(_.copy(
              nSets = 64,
              nWays = 2,
              replacementPolicy = "lru"
            ))
          ))
      }
  }) ++
  new BaselineConfig
)