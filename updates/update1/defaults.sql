DROP TABLE IF EXISTS `defaults`;
CREATE TABLE `defaults` (`id` INT NOT NULL AUTO_INCREMENT, `key` CHAR(50), `value` CHAR(50), PRIMARY KEY(`id`));
INSERT INTO `defaults` (`key`, `value`) VALUES('androidAppVer', '0'),('parserVer', '2');