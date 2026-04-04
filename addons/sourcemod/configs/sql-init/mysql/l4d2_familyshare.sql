CREATE TABLE IF NOT EXISTS `l4d2_familyshare` (
	`id` BIGINT UNSIGNED NOT NULL AUTO_INCREMENT,
	`borrower_name` VARCHAR(128) NOT NULL,
	`borrower_accountid` INT UNSIGNED NOT NULL,
	`borrower_steamid64` VARCHAR(32) NULL,
	`owner_name` VARCHAR(128) NOT NULL DEFAULT 'Unknown Owner',
	`owner_steamid64` VARCHAR(32) NULL,
	`enforced` TINYINT(1) NOT NULL DEFAULT 0,
	`created_at` TIMESTAMP NOT NULL DEFAULT CURRENT_TIMESTAMP,
	PRIMARY KEY (`id`),
	KEY `idx_l4d2_familyshare_borrower_accountid` (`borrower_accountid`),
	KEY `idx_l4d2_familyshare_owner_steamid64` (`owner_steamid64`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci;
