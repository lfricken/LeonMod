
-- Free Promotions 
INSERT INTO Technology_FreePromotions
	(TechType, PromotionType)
VALUES
	('TECH_PENICILIN', 'PROMOTION_FALLOUT_REDUCTION'),
	('TECH_NANOTECHNOLOGY', 'PROMOTION_FALLOUT_IMMUNITY'),
	('TECH_BALLISTICS', 'PROMOTION_ICE_BREAKERS');

-- Art flip metallurgy/steel
UPDATE Technologies
SET PortraitIndex = '14'
WHERE Type = 'TECH_METALLURGY' AND EXISTS (SELECT * FROM COMMUNITY WHERE Type='COMMUNITY_CORE_BALANCE_UNITS' AND Value= 1 );

-- Art flip metallurgy/steel
UPDATE Technologies
SET PortraitIndex = '33'
WHERE Type = 'TECH_STEEL' AND EXISTS (SELECT * FROM COMMUNITY WHERE Type='COMMUNITY_CORE_BALANCE_UNITS' AND Value= 1 );

UPDATE Technologies
SET PortraitIndex = '28'
WHERE Type = 'TECH_METAL_CASTING' AND EXISTS (SELECT * FROM COMMUNITY WHERE Type='COMMUNITY_CORE_BALANCE_UNITS' AND Value= 1 );

-- Art/Name flip for Construction and Masonry
UPDATE Technologies
SET Civilopedia = 'TXT_KEY_TECH_CONSTRUCTION_DESC'
WHERE Type = 'TECH_MASONRY' AND EXISTS (SELECT * FROM COMMUNITY WHERE Type='COMMUNITY_CORE_BALANCE_UNITS' AND Value= 1 );

UPDATE Technologies
SET Description = 'TXT_KEY_TECH_CONSTRUCTION_TITLE'
WHERE Type = 'TECH_MASONRY' AND EXISTS (SELECT * FROM COMMUNITY WHERE Type='COMMUNITY_CORE_BALANCE_UNITS' AND Value= 1 );

UPDATE Technologies
SET PortraitIndex = '15'
WHERE Type = 'TECH_MASONRY' AND EXISTS (SELECT * FROM COMMUNITY WHERE Type='COMMUNITY_CORE_BALANCE_UNITS' AND Value= 1 );


UPDATE Technologies
SET Civilopedia = 'TXT_KEY_TECH_MASONRY_DESC'
WHERE Type = 'TECH_CONSTRUCTION' AND EXISTS (SELECT * FROM COMMUNITY WHERE Type='COMMUNITY_CORE_BALANCE_UNITS' AND Value= 1 );

UPDATE Technologies
SET Description = 'TXT_KEY_TECH_MASONRY_TITLE'
WHERE Type = 'TECH_CONSTRUCTION' AND EXISTS (SELECT * FROM COMMUNITY WHERE Type='COMMUNITY_CORE_BALANCE_UNITS' AND Value= 1 );

UPDATE Technologies
SET PortraitIndex = '6'
WHERE Type = 'TECH_CONSTRUCTION' AND EXISTS (SELECT * FROM COMMUNITY WHERE Type='COMMUNITY_CORE_BALANCE_UNITS' AND Value= 1 );

-- AI obsoletion logic
UPDATE Resources
SET AIStopTradingEra = 'ERA_FUTURE'
WHERE Type = 'RESOURCE_IRON';

-- Move Wealth to Currency
UPDATE Processes
SET TechPrereq = 'TECH_CURRENCY'
WHERE Type = 'PROCESS_WEALTH' AND EXISTS (SELECT * FROM COMMUNITY WHERE Type='COMMUNITY_CORE_BALANCE_UNITS' AND Value= 1 );

-- Move Research to Education
UPDATE Processes
SET TechPrereq = 'TECH_EDUCATION'
WHERE Type = 'PROCESS_RESEARCH' AND EXISTS (SELECT * FROM COMMUNITY WHERE Type='COMMUNITY_CORE_BALANCE_UNITS' AND Value= 1 );

-- Move Embarkation to Sailing (Help AI) -- Move Rearrange Trade Routes

UPDATE Technologies
SET AllowsEmbarking = '0'
WHERE Type = 'TECH_OPTICS' AND EXISTS (SELECT * FROM COMMUNITY WHERE Type='COMMUNITY_CORE_BALANCE_UNITS' AND Value= 1 );

UPDATE Technologies
SET InternationalTradeRoutesChange = '0'
WHERE Type = 'TECH_ANIMAL_HUSBANDRY' AND EXISTS (SELECT * FROM COMMUNITY WHERE Type='COMMUNITY_CORE_BALANCE_UNITS' AND Value= 1 );

UPDATE Technologies
SET InternationalTradeRoutesChange = '1'
WHERE Type = 'TECH_OPTICS' AND EXISTS (SELECT * FROM COMMUNITY WHERE Type='COMMUNITY_CORE_BALANCE_UNITS' AND Value= 1 );

UPDATE Technologies
SET InternationalTradeRoutesChange = '1'
WHERE Type = 'TECH_CURRENCY' AND EXISTS (SELECT * FROM COMMUNITY WHERE Type='COMMUNITY_CORE_BALANCE_UNITS' AND Value= 1 );

UPDATE Technologies
SET AllowsEmbarking = '1'
WHERE Type = 'TECH_SAILING' AND EXISTS (SELECT * FROM COMMUNITY WHERE Type='COMMUNITY_CORE_BALANCE_UNITS' AND Value= 1 );

UPDATE Technologies
SET InternationalTradeRoutesChange = '0'
WHERE Type = 'TECH_SAILING' AND EXISTS (SELECT * FROM COMMUNITY WHERE Type='COMMUNITY_CORE_BALANCE_UNITS' AND Value= 1 );

UPDATE Technologies
SET InternationalTradeRoutesChange = '1'
WHERE Type = 'TECH_HORSEBACK_RIDING' AND EXISTS (SELECT * FROM COMMUNITY WHERE Type='COMMUNITY_CORE_BALANCE_UNITS' AND Value= 1 );

UPDATE Technologies
SET InternationalTradeRoutesChange = '0'
WHERE Type = 'TECH_ENGINEERING' AND EXISTS (SELECT * FROM COMMUNITY WHERE Type='COMMUNITY_CORE_BALANCE_UNITS' AND Value= 1 );

UPDATE Technologies
SET InternationalTradeRoutesChange = '1'
WHERE Type = 'TECH_ECONOMICS' AND EXISTS (SELECT * FROM COMMUNITY WHERE Type='COMMUNITY_CORE_BALANCE_UNITS' AND Value= 1 );

UPDATE Technologies
SET InternationalTradeRoutesChange = '0'
WHERE Type = 'TECH_BANKING' AND EXISTS (SELECT * FROM COMMUNITY WHERE Type='COMMUNITY_CORE_BALANCE_UNITS' AND Value= 1 );

UPDATE Technologies
SET InternationalTradeRoutesChange = '0'
WHERE Type = 'TECH_PENICILIN' AND EXISTS (SELECT * FROM COMMUNITY WHERE Type='COMMUNITY_CORE_BALANCE_UNITS' AND Value= 1 );

UPDATE Technologies
SET InternationalTradeRoutesChange = '1'
WHERE Type = 'TECH_COMPUTERS' AND EXISTS (SELECT * FROM COMMUNITY WHERE Type='COMMUNITY_CORE_BALANCE_UNITS' AND Value= 1 );

UPDATE Technologies
SET InternationalTradeRoutesChange = '0'
WHERE Type = 'TECH_BIOLOGY' AND EXISTS (SELECT * FROM COMMUNITY WHERE Type='COMMUNITY_CORE_BALANCE_UNITS' AND Value= 1 );

UPDATE Technologies
SET InternationalTradeRoutesChange = '1'
WHERE Type = 'TECH_ELECTRICITY' AND EXISTS (SELECT * FROM COMMUNITY WHERE Type='COMMUNITY_CORE_BALANCE_UNITS' AND Value= 1 );

-- Add Land Trade Route extension to Machinery

INSERT INTO Technology_TradeRouteDomainExtraRange
	(TechType, DomainType, Range)
VALUES
('TECH_MACHINERY', 'DOMAIN_LAND', '10');

-- City Embark Less/No Cost at Navigation, Rocketry
UPDATE Technologies
SET CityLessEmbarkCost = '1'
WHERE Type = 'TECH_NAVIGATION' AND EXISTS (SELECT * FROM COMMUNITY WHERE Type='COMMUNITY_CORE_BALANCE_UNITS' AND Value= 1 );

UPDATE Technologies
SET CityNoEmbarkCost = '1'
WHERE Type = 'TECH_ROCKETRY' AND EXISTS (SELECT * FROM COMMUNITY WHERE Type='COMMUNITY_CORE_BALANCE_UNITS' AND Value= 1 );


UPDATE Technologies
SET IconAtlas = 'COMMUNITY_ATLAS'
WHERE Type = 'TECH_SAILING' AND EXISTS (SELECT * FROM COMMUNITY WHERE Type='COMMUNITY_CORE_BALANCE_UNITS' AND Value= 1 );

UPDATE Technologies
SET PortraitIndex = '26'
WHERE Type = 'TECH_SAILING' AND EXISTS (SELECT * FROM COMMUNITY WHERE Type='COMMUNITY_CORE_BALANCE_UNITS' AND Value= 1 );

-- Rename Optics 'Sailing'

UPDATE Technologies
SET PortraitIndex = '13'
WHERE Type = 'TECH_OPTICS' AND EXISTS (SELECT * FROM COMMUNITY WHERE Type='COMMUNITY_CORE_BALANCE_UNITS' AND Value= 1 );

-- Future Tech
UPDATE Technologies
SET Happiness = '10'
WHERE Type = 'TECH_FUTURE_TECH' AND EXISTS (SELECT * FROM COMMUNITY WHERE Type='COMMUNITY_CORE_BALANCE_UNITS' AND Value= 1 );

-- Road/Railroad Speed Changes
UPDATE Routes
SET Movement = '50'
WHERE Type = 'ROUTE_ROAD';

UPDATE Routes
SET FlatMovement = '50'
WHERE Type = 'ROUTE_ROAD';

UPDATE Routes
SET Movement = '25'
WHERE Type = 'ROUTE_RAILROAD';

UPDATE Routes
SET FlatMovement = '25'
WHERE Type = 'ROUTE_RAILROAD';

-- Bombard Changes
UPDATE Technologies SET BombardRange=1 WHERE Type='TECH_AGRICULTURE';
UPDATE Technologies SET BombardRange=2 WHERE Type='TECH_MATHEMATICS';
UPDATE Technologies SET BombardRange=2, BombardIndirect=1 WHERE Type='TECH_GUNPOWDER';
UPDATE Technologies SET BombardRange=3, BombardIndirect=1 WHERE Type='TECH_DYNAMITE';

INSERT OR REPLACE INTO Defines(Name, Value) SELECT 'MAX_CITY_ATTACK_RANGE', max(BombardRange) FROM Technologies;

UPDATE CustomModOptions SET Value=1 WHERE Name='EVENTS_CITY_BOMBARD';

-- TR Food/Production Scaling

UPDATE Eras
SET TradeRouteFoodBonusTimes100 = '100'
WHERE Type = 'ERA_CLASSICAL';

UPDATE Eras
SET TradeRouteProductionBonusTimes100 = '100'
WHERE Type = 'ERA_CLASSICAL';

UPDATE Eras
SET TradeRouteFoodBonusTimes100 = '200'
WHERE Type = 'ERA_MEDIEVAL';

UPDATE Eras
SET TradeRouteProductionBonusTimes100 = '200'
WHERE Type = 'ERA_MEDIEVAL';

UPDATE Eras
SET TradeRouteFoodBonusTimes100 = '400'
WHERE Type = 'ERA_RENAISSANCE';

UPDATE Eras
SET TradeRouteProductionBonusTimes100 = '400'
WHERE Type = 'ERA_RENAISSANCE';

UPDATE Eras
SET TradeRouteFoodBonusTimes100 = '600'
WHERE Type = 'ERA_INDUSTRIAL';

UPDATE Eras
SET TradeRouteProductionBonusTimes100 = '600'
WHERE Type = 'ERA_INDUSTRIAL';

UPDATE Eras
SET TradeRouteFoodBonusTimes100 = '800'
WHERE Type = 'ERA_MODERN';

UPDATE Eras
SET TradeRouteProductionBonusTimes100 = '800'
WHERE Type = 'ERA_MODERN';

UPDATE Eras
SET TradeRouteFoodBonusTimes100 = '1000'
WHERE Type = 'ERA_POSTMODERN';

UPDATE Eras
SET TradeRouteProductionBonusTimes100 = '1000'
WHERE Type = 'ERA_POSTMODERN';

UPDATE Eras
SET TradeRouteFoodBonusTimes100 = '1200'
WHERE Type = 'ERA_FUTURE';

UPDATE Eras
SET TradeRouteProductionBonusTimes100 = '1200'
WHERE Type = 'ERA_FUTURE';

-- Tech Prereqs

-- Delete Prereqs
DELETE FROM Technology_PrereqTechs;

INSERT INTO Technology_PrereqTechs
	(TechType, PrereqTech)
VALUES
	('TECH_FUTURE_TECH', 'TECH_NANOTECHNOLOGY'),
	('TECH_FUTURE_TECH', 'TECH_PARTICLE_PHYSICS'),
	('TECH_FUTURE_TECH', 'TECH_NUCLEAR_FUSION'),
	('TECH_FUTURE_TECH', 'TECH_GLOBALIZATION'),
	('TECH_NANOTECHNOLOGY', 'TECH_ROBOTICS'),
	('TECH_NANOTECHNOLOGY', 'TECH_INTERNET'),
	('TECH_PARTICLE_PHYSICS', 'TECH_INTERNET'),
	('TECH_PARTICLE_PHYSICS', 'TECH_LASERS'),
	('TECH_PARTICLE_PHYSICS', 'TECH_ROBOTICS'),
	('TECH_GLOBALIZATION', 'TECH_LASERS'),
	('TECH_GLOBALIZATION', 'TECH_STEALTH'),
	('TECH_GLOBALIZATION', 'TECH_INTERNET'),
	('TECH_NUCLEAR_FUSION', 'TECH_STEALTH'),
	('TECH_NUCLEAR_FUSION', 'TECH_LASERS'),
	('TECH_ROBOTICS', 'TECH_ECOLOGY'),
	('TECH_ROBOTICS', 'TECH_TELECOM'),
	('TECH_LASERS', 'TECH_ADVANCED_BALLISTICS'),
	('TECH_LASERS', 'TECH_SATELLITES'),
	('TECH_INTERNET', 'TECH_SATELLITES'),
	('TECH_INTERNET', 'TECH_TELECOM'),
	('TECH_STEALTH', 'TECH_ADVANCED_BALLISTICS'),
	('TECH_STEALTH', 'TECH_MOBILE_TACTICS'),
	('TECH_ECOLOGY', 'TECH_ELECTRONICS'),
	('TECH_TELECOM', 'TECH_ELECTRONICS'),
	('TECH_TELECOM', 'TECH_COMPUTERS'),
	('TECH_SATELLITES', 'TECH_COMPUTERS'),
	('TECH_SATELLITES', 'TECH_NUCLEAR_FISSION'),
	('TECH_ADVANCED_BALLISTICS', 'TECH_NUCLEAR_FISSION'),
	('TECH_ADVANCED_BALLISTICS', 'TECH_RADAR'),
	('TECH_MOBILE_TACTICS', 'TECH_RADAR'),
	('TECH_ELECTRONICS', 'TECH_PENICILIN'),
	('TECH_ELECTRONICS', 'TECH_REFRIGERATION'),
	('TECH_COMPUTERS', 'TECH_ATOMIC_THEORY'),
	('TECH_COMPUTERS', 'TECH_REFRIGERATION'),
	('TECH_NUCLEAR_FISSION', 'TECH_ATOMIC_THEORY'),
	('TECH_NUCLEAR_FISSION', 'TECH_ROCKETRY'),
	('TECH_RADAR', 'TECH_ROCKETRY'),
	('TECH_RADAR', 'TECH_COMBINED_ARMS'),
	('TECH_PENICILIN', 'TECH_PLASTIC'),
	('TECH_REFRIGERATION', 'TECH_PLASTIC'),
	('TECH_REFRIGERATION', 'TECH_RADIO'),
	('TECH_ATOMIC_THEORY', 'TECH_FLIGHT'),
	('TECH_ATOMIC_THEORY', 'TECH_RADIO'),
	('TECH_COMBINED_ARMS', 'TECH_BALLISTICS'),
	('TECH_ROCKETRY', 'TECH_FLIGHT'),
	('TECH_ROCKETRY', 'TECH_BALLISTICS'),
	('TECH_PLASTIC', 'TECH_BIOLOGY'),
	('TECH_PLASTIC', 'TECH_ELECTRICITY'),
	('TECH_RADIO', 'TECH_ELECTRICITY'),
	('TECH_RADIO', 'TECH_REPLACEABLE_PARTS'),
	('TECH_RADIO', 'TECH_BIOLOGY'),
	('TECH_FLIGHT', 'TECH_COMBUSTION'),
	('TECH_FLIGHT', 'TECH_REPLACEABLE_PARTS'),
	('TECH_FLIGHT', 'TECH_ELECTRICITY'),
	('TECH_BALLISTICS', 'TECH_COMBUSTION'),
	('TECH_BALLISTICS', 'TECH_REPLACEABLE_PARTS'),
	('TECH_BIOLOGY', 'TECH_ARCHAEOLOGY'),
	('TECH_BIOLOGY', 'TECH_FERTILIZER'),
	('TECH_ELECTRICITY', 'TECH_FERTILIZER'),
	('TECH_ELECTRICITY', 'TECH_INDUSTRIALIZATION'),
	('TECH_REPLACEABLE_PARTS', 'TECH_INDUSTRIALIZATION'),
	('TECH_REPLACEABLE_PARTS', 'TECH_DYNAMITE'),
	('TECH_COMBUSTION', 'TECH_DYNAMITE'),
	('TECH_COMBUSTION', 'TECH_MILITARY_SCIENCE'),
	('TECH_ARCHAEOLOGY', 'TECH_SCIENTIFIC_THEORY'),
	('TECH_FERTILIZER', 'TECH_SCIENTIFIC_THEORY'),
	('TECH_FERTILIZER', 'TECH_RAILROAD'),
	('TECH_INDUSTRIALIZATION', 'TECH_RAILROAD'),
	('TECH_INDUSTRIALIZATION', 'TECH_STEAM_POWER'),
	('TECH_DYNAMITE', 'TECH_STEAM_POWER'),
	('TECH_DYNAMITE', 'TECH_RIFLING'),
	('TECH_MILITARY_SCIENCE', 'TECH_RIFLING'),
	('TECH_SCIENTIFIC_THEORY', 'TECH_ECONOMICS'),
	('TECH_SCIENTIFIC_THEORY', 'TECH_ARCHITECTURE'),
	('TECH_RAILROAD', 'TECH_ACOUSTICS'),
	('TECH_RAILROAD', 'TECH_ECONOMICS'),
	('TECH_STEAM_POWER', 'TECH_NAVIGATION'),
	('TECH_STEAM_POWER', 'TECH_ACOUSTICS'),
	('TECH_RIFLING', 'TECH_NAVIGATION'),
	('TECH_RIFLING', 'TECH_METALLURGY'),
	('TECH_ARCHITECTURE', 'TECH_BANKING'),
	('TECH_ARCHITECTURE', 'TECH_PRINTING_PRESS'),
	('TECH_ECONOMICS', 'TECH_PRINTING_PRESS'),
	('TECH_ECONOMICS', 'TECH_ASTRONOMY'),
	('TECH_ECONOMICS', 'TECH_BANKING'),
	('TECH_ACOUSTICS', 'TECH_ASTRONOMY'),
	('TECH_ACOUSTICS', 'TECH_PRINTING_PRESS'),
	('TECH_ACOUSTICS', 'TECH_GUNPOWDER'),
	('TECH_NAVIGATION', 'TECH_GUNPOWDER'),
	('TECH_NAVIGATION', 'TECH_CHEMISTRY'),
	('TECH_NAVIGATION', 'TECH_ASTRONOMY'),
	('TECH_METALLURGY', 'TECH_CHEMISTRY'),
	('TECH_METALLURGY', 'TECH_GUNPOWDER'),
	('TECH_BANKING', 'TECH_CIVIL_SERVICE'),
	('TECH_PRINTING_PRESS', 'TECH_CIVIL_SERVICE'),
	('TECH_PRINTING_PRESS', 'TECH_GUILDS'),
	('TECH_ASTRONOMY', 'TECH_GUILDS'),
	('TECH_ASTRONOMY', 'TECH_COMPASS'),
	('TECH_GUNPOWDER', 'TECH_COMPASS'),
	('TECH_GUNPOWDER', 'TECH_MACHINERY'),
	('TECH_CHEMISTRY', 'TECH_MACHINERY'),
	('TECH_CIVIL_SERVICE', 'TECH_THEOLOGY'),
	('TECH_CIVIL_SERVICE', 'TECH_EDUCATION'),
	('TECH_GUILDS', 'TECH_THEOLOGY'),
	('TECH_GUILDS', 'TECH_CHIVALRY'),
	('TECH_COMPASS', 'TECH_PHYSICS'),
	('TECH_COMPASS', 'TECH_CHIVALRY'),
	('TECH_MACHINERY', 'TECH_STEEL'),
	('TECH_MACHINERY', 'TECH_PHYSICS'),
	('TECH_EDUCATION', 'TECH_DRAMA'),
	('TECH_EDUCATION', 'TECH_PHILOSOPHY'),
	('TECH_THEOLOGY', 'TECH_PHILOSOPHY'),
	('TECH_THEOLOGY', 'TECH_DRAMA'),
	('TECH_THEOLOGY', 'TECH_ENGINEERING'),
	('TECH_CHIVALRY', 'TECH_CURRENCY'),
	('TECH_CHIVALRY', 'TECH_ENGINEERING'),
	('TECH_CHIVALRY', 'TECH_PHILOSOPHY'),
	('TECH_PHYSICS', 'TECH_ENGINEERING'),
	('TECH_PHYSICS', 'TECH_CURRENCY'),
	('TECH_PHYSICS', 'TECH_METAL_CASTING'),
	('TECH_STEEL', 'TECH_METAL_CASTING'),
	('TECH_STEEL', 'TECH_CURRENCY'),
	('TECH_DRAMA', 'TECH_OPTICS'),
	('TECH_DRAMA', 'TECH_WRITING'),
	('TECH_PHILOSOPHY', 'TECH_WRITING'),
	('TECH_PHILOSOPHY', 'TECH_MATHEMATICS'),
	('TECH_PHILOSOPHY', 'TECH_OPTICS'),
	('TECH_ENGINEERING', 'TECH_MATHEMATICS'),
	('TECH_ENGINEERING', 'TECH_CONSTRUCTION'),
	('TECH_ENGINEERING', 'TECH_WRITING'),
	('TECH_CURRENCY', 'TECH_MATHEMATICS'),
	('TECH_CURRENCY', 'TECH_IRON_WORKING'),
	('TECH_CURRENCY', 'TECH_CONSTRUCTION'),
	('TECH_METAL_CASTING', 'TECH_CONSTRUCTION'),
	('TECH_METAL_CASTING', 'TECH_IRON_WORKING'),
	('TECH_OPTICS', 'TECH_SAILING'),
	('TECH_OPTICS', 'TECH_HORSEBACK_RIDING'),
	('TECH_WRITING', 'TECH_CALENDAR'),
	('TECH_WRITING', 'TECH_HORSEBACK_RIDING'),
	('TECH_MATHEMATICS', 'TECH_ARCHERY'),
	('TECH_MATHEMATICS', 'TECH_CALENDAR'),
	('TECH_CONSTRUCTION', 'TECH_ARCHERY'),
	('TECH_CONSTRUCTION', 'TECH_MASONRY'),
	('TECH_IRON_WORKING', 'TECH_BRONZE_WORKING'),
	('TECH_IRON_WORKING', 'TECH_MASONRY'),
	('TECH_SAILING', 'TECH_TRAPPING'),
	('TECH_ARCHERY', 'TECH_THE_WHEEL'),
	('TECH_ARCHERY', 'TECH_ANIMAL_HUSBANDRY'),
	('TECH_HORSEBACK_RIDING', 'TECH_TRAPPING'),
	('TECH_HORSEBACK_RIDING', 'TECH_POTTERY'),
	('TECH_CALENDAR', 'TECH_THE_WHEEL'),
	('TECH_CALENDAR', 'TECH_POTTERY'),
	('TECH_MASONRY', 'TECH_ANIMAL_HUSBANDRY'),
	('TECH_MASONRY', 'TECH_MINING'),
	('TECH_BRONZE_WORKING', 'TECH_MINING'),
	('TECH_TRAPPING', 'TECH_AGRICULTURE'),
	('TECH_ANIMAL_HUSBANDRY', 'TECH_AGRICULTURE'),
	('TECH_THE_WHEEL', 'TECH_AGRICULTURE'),
	('TECH_POTTERY', 'TECH_AGRICULTURE'),
	('TECH_MINING', 'TECH_AGRICULTURE');

	-- And Flavors
	DELETE FROM Technology_Flavors;

	INSERT INTO Technology_Flavors
	(TechType, FlavorType, Flavor)
VALUES
	('TECH_AGRICULTURE', 'FLAVOR_GROWTH', 10),
	('TECH_POTTERY', 'FLAVOR_GROWTH', 20),
	('TECH_POTTERY', 'FLAVOR_EXPANSION', 25),
	('TECH_POTTERY', 'FLAVOR_RELIGION', 10),
	('TECH_POTTERY', 'FLAVOR_WONDER', 10),
	('TECH_ANIMAL_HUSBANDRY', 'FLAVOR_TILE_IMPROVEMENT', 25),
	('TECH_ANIMAL_HUSBANDRY', 'FLAVOR_GROWTH', 10),
	('TECH_ARCHERY', 'FLAVOR_MILITARY_TRAINING', 10),
	('TECH_ARCHERY', 'FLAVOR_OFFENSE', 15),
	('TECH_ARCHERY', 'FLAVOR_MOBILE', 10),
	('TECH_ARCHERY', 'FLAVOR_WONDER', 10),
	('TECH_MINING', 'FLAVOR_PRODUCTION', 10),
	('TECH_MINING', 'FLAVOR_GROWTH', 10),
	('TECH_MINING', 'FLAVOR_TILE_IMPROVEMENT', 10),
	('TECH_SAILING', 'FLAVOR_NAVAL', 10),
	('TECH_SAILING', 'FLAVOR_NAVAL_TILE_IMPROVEMENT', 15),
	('TECH_SAILING', 'FLAVOR_EXPANSION', 5),
	('TECH_CALENDAR', 'FLAVOR_TILE_IMPROVEMENT', 15),
	('TECH_CALENDAR', 'FLAVOR_WONDER', 10),
	('TECH_CALENDAR', 'FLAVOR_EXPANSION', 10),
	('TECH_CALENDAR', 'FLAVOR_GROWTH', 5),
	('TECH_WRITING', 'FLAVOR_SCIENCE', 15),
	('TECH_WRITING', 'FLAVOR_WONDER', 10),
	('TECH_WRITING', 'FLAVOR_DIPLOMACY', 10),
	('TECH_TRAPPING', 'FLAVOR_RANGED', 10),
	('TECH_TRAPPING', 'FLAVOR_TILE_IMPROVEMENT', 15),
	('TECH_THE_WHEEL', 'FLAVOR_TILE_IMPROVEMENT', 5),
	('TECH_THE_WHEEL', 'FLAVOR_RANGED', 5),
	('TECH_THE_WHEEL', 'FLAVOR_INFRASTRUCTURE', 10),
	('TECH_THE_WHEEL', 'FLAVOR_MOBILE', 10),
	('TECH_THE_WHEEL', 'FLAVOR_SCIENCE', 10),
	('TECH_MASONRY', 'FLAVOR_CITY_DEFENSE', 10),
	('TECH_MASONRY', 'FLAVOR_WONDER', 5),
	('TECH_MASONRY', 'FLAVOR_TILE_IMPROVEMENT', 10),
	('TECH_MASONRY', 'FLAVOR_WONDER', 10),
	('TECH_BRONZE_WORKING', 'FLAVOR_DEFENSE', 15),
	('TECH_BRONZE_WORKING', 'FLAVOR_MILITARY_TRAINING', 5),
	('TECH_BRONZE_WORKING', 'FLAVOR_TILE_IMPROVEMENT', 10),
	('TECH_OPTICS', 'FLAVOR_NAVAL', 15),
	('TECH_OPTICS', 'FLAVOR_NAVAL_GROWTH', 15),
	('TECH_OPTICS', 'FLAVOR_NAVAL_RECON', 15),
	('TECH_OPTICS', 'FLAVOR_GROWTH', 15),
	('TECH_OPTICS', 'FLAVOR_EXPANSION', 15),
	('TECH_OPTICS', 'FLAVOR_I_SEA_TRADE_ROUTE', 25),
	('TECH_HORSEBACK_RIDING', 'FLAVOR_GOLD', 25),
	('TECH_HORSEBACK_RIDING', 'FLAVOR_WONDER', 10),
	('TECH_HORSEBACK_RIDING', 'FLAVOR_I_LAND_TRADE_ROUTE', 25),
	('TECH_MATHEMATICS', 'FLAVOR_RANGED', 10),
	('TECH_MATHEMATICS', 'FLAVOR_MOBILE', 10),
	('TECH_MATHEMATICS', 'FLAVOR_WONDER', 10),
	('TECH_MATHEMATICS', 'FLAVOR_OFFENSE', 15),
	('TECH_CONSTRUCTION', 'FLAVOR_PRODUCTION', 10),
	('TECH_CONSTRUCTION', 'FLAVOR_DEFENSE', 5),
	('TECH_CONSTRUCTION', 'FLAVOR_WONDER', 10),
	('TECH_CONSTRUCTION', 'FLAVOR_TILE_IMPROVEMENT', 10),
	('TECH_PHILOSOPHY', 'FLAVOR_SCIENCE', 10),
	('TECH_PHILOSOPHY', 'FLAVOR_CULTURE', 15),
	('TECH_PHILOSOPHY', 'FLAVOR_RELIGION', 10),
	('TECH_PHILOSOPHY', 'FLAVOR_WONDER', 15),
	('TECH_DRAMA', 'FLAVOR_CULTURE', 15),
	('TECH_DRAMA', 'FLAVOR_GREAT_PEOPLE', 5),
	('TECH_DRAMA', 'FLAVOR_WONDER', 5),
	('TECH_CURRENCY', 'FLAVOR_GOLD', 20),
	('TECH_CURRENCY', 'FLAVOR_TILE_IMPROVEMENT', 10),
	('TECH_IRON_WORKING', 'FLAVOR_OFFENSE', 15),
	('TECH_IRON_WORKING', 'FLAVOR_DEFENSE', 10),
	('TECH_IRON_WORKING', 'FLAVOR_PRODUCTION', 10),
	('TECH_IRON_WORKING', 'FLAVOR_MILITARY_TRAINING', 10),
	('TECH_THEOLOGY', 'FLAVOR_RELIGION', 15),
	('TECH_THEOLOGY', 'FLAVOR_GREAT_PEOPLE', 5),
	('TECH_THEOLOGY', 'FLAVOR_WONDER', 10),
	('TECH_CIVIL_SERVICE', 'FLAVOR_TILE_IMPROVEMENT', 15),
	('TECH_CIVIL_SERVICE', 'FLAVOR_GROWTH', 15),
	('TECH_CIVIL_SERVICE', 'FLAVOR_PRODUCTION', 15),
	('TECH_CIVIL_SERVICE', 'FLAVOR_WONDER', 10),
	('TECH_GUILDS', 'FLAVOR_GOLD', 25),
	('TECH_GUILDS', 'FLAVOR_WONDER', 10),
	('TECH_GUILDS', 'FLAVOR_NAVAL', 10),
	('TECH_ENGINEERING', 'FLAVOR_DEFENSE', 10),
	('TECH_ENGINEERING', 'FLAVOR_GROWTH', 15),
	('TECH_ENGINEERING', 'FLAVOR_RANGED', 10),
	('TECH_ENGINEERING', 'FLAVOR_INFRASTRUCTURE', 10),
	('TECH_METAL_CASTING', 'FLAVOR_PRODUCTION', 25),
	('TECH_METAL_CASTING', 'FLAVOR_WONDER', 10),
	('TECH_METAL_CASTING', 'FLAVOR_PRODUCTION', 10),
	('TECH_METAL_CASTING', 'FLAVOR_CULTURE', 5),
	('TECH_METAL_CASTING', 'FLAVOR_HAPPINESS', 10),
	('TECH_COMPASS', 'FLAVOR_PRODUCTION', 5),
	('TECH_COMPASS', 'FLAVOR_NAVAL', 15),
	('TECH_COMPASS', 'FLAVOR_WATER_CONNECTION', 10),
	('TECH_EDUCATION', 'FLAVOR_SCIENCE', 25),
	('TECH_EDUCATION', 'FLAVOR_WONDER', 15),
	('TECH_EDUCATION', 'FLAVOR_DIPLOMACY', 10),
	('TECH_CHIVALRY', 'FLAVOR_OFFENSE', 25),
	('TECH_CHIVALRY', 'FLAVOR_MOBILE', 15),
	('TECH_CHIVALRY', 'FLAVOR_CITY_DEFENSE', 15),
	('TECH_CHIVALRY', 'FLAVOR_WONDER', 10),
	('TECH_CHIVALRY', 'FLAVOR_DIPLOMACY', 5),
	('TECH_MACHINERY', 'FLAVOR_RANGED', 10),
	('TECH_MACHINERY', 'FLAVOR_INFRASTRUCTURE', 10),
	('TECH_MACHINERY', 'FLAVOR_PRODUCTION', 15),
	('TECH_MACHINERY', 'FLAVOR_TILE_IMPROVEMENT', 10),
	('TECH_MACHINERY', 'FLAVOR_DEFENSE', 5),
	('TECH_PHYSICS', 'FLAVOR_RANGED', 10),
	('TECH_PHYSICS', 'FLAVOR_MOBILE', 10),
	('TECH_PHYSICS', 'FLAVOR_WONDER', 5),
	('TECH_PHYSICS', 'FLAVOR_HAPPINESS', 5),
	('TECH_STEEL', 'FLAVOR_OFFENSE', 20),
	('TECH_STEEL', 'FLAVOR_DEFENSE', 15),
	('TECH_STEEL', 'FLAVOR_MILITARY_TRAINING', 10),
	('TECH_ASTRONOMY', 'FLAVOR_NAVAL_RECON', 15),
	('TECH_ASTRONOMY', 'FLAVOR_RECON', 10),
	('TECH_ASTRONOMY', 'FLAVOR_NAVAL', 10),
	('TECH_ASTRONOMY', 'FLAVOR_SCIENCE', 15),
	('TECH_ASTRONOMY', 'FLAVOR_EXPANSION', 15),
	('TECH_ACOUSTICS', 'FLAVOR_CULTURE', 15),
	('TECH_ACOUSTICS', 'FLAVOR_WONDER', 10),
	('TECH_BANKING', 'FLAVOR_GOLD', 25),
	('TECH_BANKING', 'FLAVOR_EXPANSION', 5),
	('TECH_BANKING', 'FLAVOR_TILE_IMPROVEMENT', 5),
	('TECH_BANKING', 'FLAVOR_WONDER', 10),
	('TECH_PRINTING_PRESS', 'FLAVOR_CULTURE', 10),
	('TECH_PRINTING_PRESS', 'FLAVOR_WONDER', 10),
	('TECH_PRINTING_PRESS', 'FLAVOR_GREAT_PEOPLE', 5),
	('TECH_GUNPOWDER', 'FLAVOR_DEFENSE', 15),
	('TECH_GUNPOWDER', 'FLAVOR_OFFENSE', 25),
	('TECH_GUNPOWDER', 'FLAVOR_WONDER', 5),
	('TECH_NAVIGATION', 'FLAVOR_NAVAL', 25),
	('TECH_NAVIGATION', 'FLAVOR_NAVAL_TILE_IMPROVEMENT', 5),
	('TECH_ARCHITECTURE', 'FLAVOR_WONDER', 25),
	('TECH_ARCHITECTURE', 'FLAVOR_CULTURE', 15),
	('TECH_ECONOMICS', 'FLAVOR_GOLD', 15),
	('TECH_ECONOMICS', 'FLAVOR_PRODUCTION', 15),
	('TECH_CHEMISTRY', 'FLAVOR_GROWTH', 15),
	('TECH_METALLURGY', 'FLAVOR_OFFENSE', 15),
	('TECH_METALLURGY', 'FLAVOR_MOBILE', 15),
	('TECH_METALLURGY', 'FLAVOR_DEFENSE', 15),
	('TECH_ARCHAEOLOGY', 'FLAVOR_WONDER', 10),
	('TECH_ARCHAEOLOGY', 'FLAVOR_CULTURE', 25),
	('TECH_ARCHAEOLOGY', 'FLAVOR_WONDER', 25),
	('TECH_SCIENTIFIC_THEORY', 'FLAVOR_SCIENCE', 25),
	('TECH_SCIENTIFIC_THEORY', 'FLAVOR_CULTURE', 5),
	('TECH_INDUSTRIALIZATION', 'FLAVOR_PRODUCTION', 25),
	('TECH_INDUSTRIALIZATION', 'FLAVOR_WONDER', 10),
	('TECH_INDUSTRIALIZATION', 'FLAVOR_DIPLOMACY', 10),
	('TECH_INDUSTRIALIZATION', 'FLAVOR_NAVAL', 10),
	('TECH_MILITARY_SCIENCE', 'FLAVOR_OFFENSE', 10),
	('TECH_MILITARY_SCIENCE', 'FLAVOR_MOBILE', 15),
	('TECH_MILITARY_SCIENCE', 'FLAVOR_MILITARY_TRAINING', 15),
	('TECH_MILITARY_SCIENCE', 'FLAVOR_WONDER', 5),
	('TECH_FERTILIZER', 'FLAVOR_GROWTH', 30),
	('TECH_FERTILIZER', 'FLAVOR_TILE_IMPROVEMENT', 10),
	('TECH_RIFLING', 'FLAVOR_DEFENSE', 10),
	('TECH_RIFLING', 'FLAVOR_OFFENSE', 25),
	('TECH_RIFLING', 'FLAVOR_CITY_DEFENSE', 5),
	('TECH_BIOLOGY', 'FLAVOR_GROWTH', 25),
	('TECH_BIOLOGY', 'FLAVOR_TILE_IMPROVEMENT', 5),
	('TECH_STEAM_POWER', 'FLAVOR_NAVAL', 15),
	('TECH_STEAM_POWER', 'FLAVOR_TILE_IMPROVEMENT', 10),
	('TECH_STEAM_POWER', 'FLAVOR_PRODUCTION', 15),
	('TECH_ELECTRICITY', 'FLAVOR_GOLD', 15),
	('TECH_ELECTRICITY', 'FLAVOR_WONDER', 5),
	('TECH_ELECTRICITY', 'FLAVOR_TILE_IMPROVEMENT', 10),
	('TECH_ELECTRICITY', 'FLAVOR_PRODUCTION', 5),
	('TECH_REPLACEABLE_PARTS', 'FLAVOR_OFFENSE', 15),
	('TECH_REPLACEABLE_PARTS', 'FLAVOR_WONDER', 5),
	('TECH_REPLACEABLE_PARTS', 'FLAVOR_DIPLOMACY', 5),
	('TECH_RAILROAD', 'FLAVOR_INFRASTRUCTURE', 10),
	('TECH_RAILROAD', 'FLAVOR_PRODUCTION', 10),
	('TECH_RAILROAD', 'FLAVOR_WONDER', 10),
	('TECH_RAILROAD', 'FLAVOR_CULTURE', 10),
	('TECH_DYNAMITE', 'FLAVOR_RANGED', 20),
	('TECH_DYNAMITE', 'FLAVOR_NAVAL', 10),
	('TECH_DYNAMITE', 'FLAVOR_OFFENSE', 10),
	('TECH_REFRIGERATION', 'FLAVOR_NAVAL_TILE_IMPROVEMENT', 5),
	('TECH_REFRIGERATION', 'FLAVOR_PRODUCTION', 5),
	('TECH_REFRIGERATION', 'FLAVOR_CULTURE', 10),
	('TECH_REFRIGERATION', 'FLAVOR_TILE_IMPROVEMENT', 5),
	('TECH_RADIO', 'FLAVOR_CULTURE', 25),
	('TECH_RADIO', 'FLAVOR_WONDER', 5),
	('TECH_FLIGHT', 'FLAVOR_AIR', 25),
	('TECH_FLIGHT', 'FLAVOR_OFFENSE', 20),
	('TECH_FLIGHT', 'FLAVOR_CULTURE', 5),
	('TECH_BALLISTICS', 'FLAVOR_OFFENSE', 10),
	('TECH_BALLISTICS', 'FLAVOR_ANTIAIR', 5),
	('TECH_BALLISTICS', 'FLAVOR_DEFENSE', 10),
	('TECH_COMBUSTION', 'FLAVOR_OFFENSE', 15),
	('TECH_COMBUSTION', 'FLAVOR_MOBILE', 5),
	('TECH_COMBUSTION', 'FLAVOR_TILE_IMPROVEMENT', 10),
	('TECH_PLASTIC', 'FLAVOR_CULTURE', 5),
	('TECH_PLASTIC', 'FLAVOR_WONDER', 10),
	('TECH_PLASTIC', 'FLAVOR_SCIENCE', 20),
	('TECH_PENICILIN', 'FLAVOR_GROWTH', 10),
	('TECH_PENICILIN', 'FLAVOR_NAVAL', 5),
	('TECH_PENICILIN', 'FLAVOR_OFFENSE', 5),
	('TECH_ELECTRONICS', 'FLAVOR_NAVAL', 10),
	('TECH_ELECTRONICS', 'FLAVOR_OFFENSE', 5),
	('TECH_ELECTRONICS', 'FLAVOR_PRODUCTION', 10),
	('TECH_RADAR', 'FLAVOR_AIR', 25),
	('TECH_RADAR', 'FLAVOR_OFFENSE', 10),
	('TECH_RADAR', 'FLAVOR_CULTURE', 10),
	('TECH_COMBINED_ARMS', 'FLAVOR_OFFENSE', 15),
	('TECH_COMBINED_ARMS', 'FLAVOR_MOBILE', 15),
	('TECH_COMBINED_ARMS', 'FLAVOR_DEFENSE', 5),
	('TECH_COMBINED_ARMS', 'FLAVOR_WONDER', 5),
	('TECH_ATOMIC_THEORY', 'FLAVOR_NUKE', 25),
	('TECH_ATOMIC_THEORY', 'FLAVOR_OFFENSE', 10),
	('TECH_ATOMIC_THEORY', 'FLAVOR_DIPLOMACY', 5),
	('TECH_ECOLOGY', 'FLAVOR_PRODUCTION', 10),
	('TECH_ECOLOGY', 'FLAVOR_CULTURE', 10),
	('TECH_COMPUTERS', 'FLAVOR_AIR_CARRIER', 5),
	('TECH_COMPUTERS', 'FLAVOR_NAVAL', 10),
	('TECH_COMPUTERS', 'FLAVOR_ESPIONAGE', 5),
	('TECH_TELECOM', 'FLAVOR_WONDER', 15),
	('TECH_TELECOM', 'FLAVOR_CULTURE', 15),
	('TECH_ROCKETRY', 'FLAVOR_RANGED', 10),
	('TECH_ROCKETRY', 'FLAVOR_AIR', 5),
	('TECH_ROCKETRY', 'FLAVOR_MOBILE', 5),
	('TECH_ROCKETRY', 'FLAVOR_SPACESHIP', 30),
	('TECH_LASERS', 'FLAVOR_OFFENSE', 5),
	('TECH_LASERS', 'FLAVOR_MOBILE', 10),
	('TECH_LASERS', 'FLAVOR_AIR', 10),
	('TECH_NUCLEAR_FISSION', 'FLAVOR_NUKE', 30),
	('TECH_NUCLEAR_FISSION', 'FLAVOR_DEFENSE', 10),
	('TECH_NUCLEAR_FISSION', 'FLAVOR_PRODUCTION', 10),
	('TECH_INTERNET', 'FLAVOR_CULTURE', 50),
	('TECH_GLOBALIZATION', 'FLAVOR_DIPLOMACY', 25),
	('TECH_GLOBALIZATION', 'FLAVOR_SPACESHIP', 25),
	('TECH_ROBOTICS', 'FLAVOR_SPACESHIP', 15),
	('TECH_ROBOTICS', 'FLAVOR_NAVAL', 15),
	('TECH_SATELLITES', 'FLAVOR_SCIENCE', 30),
	('TECH_SATELLITES', 'FLAVOR_WONDER', 15),
	('TECH_SATELLITES', 'FLAVOR_OFFENSE', 15),
	('TECH_SATELLITES', 'FLAVOR_SPACESHIP', 75),
	('TECH_STEALTH', 'FLAVOR_AIR', 10),
	('TECH_STEALTH', 'FLAVOR_NAVAL', 10),
	('TECH_MOBILE_TACTICS', 'FLAVOR_MOBILE', 15),
	('TECH_MOBILE_TACTICS', 'FLAVOR_OFFENSE', 15),
	('TECH_ADVANCED_BALLISTICS', 'FLAVOR_NUKE', 10),
	('TECH_ADVANCED_BALLISTICS', 'FLAVOR_DEFENSE', 10),
	('TECH_ADVANCED_BALLISTICS', 'FLAVOR_AIR', 10),
	('TECH_ADVANCED_BALLISTICS', 'FLAVOR_ANTIAIR', 10),
	('TECH_PARTICLE_PHYSICS', 'FLAVOR_SPACESHIP', 20),
	('TECH_PARTICLE_PHYSICS', 'FLAVOR_WONDER', 20),
	('TECH_NUCLEAR_FUSION', 'FLAVOR_OFFENSE', 20),
	('TECH_NUCLEAR_FUSION', 'FLAVOR_SPACESHIP', 20),
	('TECH_NANOTECHNOLOGY', 'FLAVOR_SPACESHIP', 20),
	('TECH_NANOTECHNOLOGY', 'FLAVOR_OFFENSE', 20),
	('TECH_FUTURE_TECH', 'FLAVOR_SCIENCE', 10);