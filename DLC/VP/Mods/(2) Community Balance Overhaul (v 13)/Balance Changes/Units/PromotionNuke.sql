-- Delete Bad Stuff
	DELETE FROM UnitPromotions
	WHERE Type = 'PROMOTION_RECON_SHORT_RANGE';

	DELETE FROM UnitPromotions
	WHERE Type = 'PROMOTION_RECON_LONG_RANGE';
-- THIS NEEDS TO BE FIRST, SILLY.

	