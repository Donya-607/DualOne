#pragma once

namespace Music
{
	enum ID
	{
		BGM_Clear,
		BGM_Game,
		BGM_Over,
		BGM_Title,

		BossBeamCharge,
		BossBeamShoot,
		BossEngine,
		BossDefeated,
		BossImpact,
		BossReceiveDamage,
		BossRushWave,
		BossShootMissile,

		PlayerCharge,
		PlayerChargeComplete,
		PlayerReflect,
		PlayerSlipping,
		PlayerTumble,
		PlayerJump,
		PlayerLaneMove = PlayerJump,

		ObjBreakObstacle,

		ItemChoose,
		ItemDecision,

		TERMINATION_OF_MUSIC_ID
	};
}
