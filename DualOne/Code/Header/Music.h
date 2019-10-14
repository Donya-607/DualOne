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

		ItemChoose,
		ItemDecision,

		TERMINATION_OF_MUSIC_ID
	};
}
