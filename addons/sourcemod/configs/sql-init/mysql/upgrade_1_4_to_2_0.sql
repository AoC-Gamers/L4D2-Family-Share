-- Run once for an existing 1.4 table. Substitute its name if configured.
-- Historical rows retain unknown owner_accountid=0; no inferred permissions.
ALTER TABLE `l4d2_familyshare`
	ADD COLUMN `owner_accountid` INT UNSIGNED NOT NULL DEFAULT 0 AFTER `owner_name`,
	ADD COLUMN `allowed` TINYINT(1) NOT NULL DEFAULT 0 AFTER `owner_steamid64`,
	ADD KEY `idx_l4d2_familyshare_owner_accountid` (`owner_accountid`);
