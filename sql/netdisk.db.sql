drop database if exists pan;
create database pan;
use pan;
create table FileFragmentEntity(
	`id` int primary key AUTO_INCREMENT,
    `MD5` varchar(255) not null,
    `fgsize` int not null,
    `link_num` int not null,
    constraint FFE_link_num check (`link_num` >=0 and `fgsize` >=0)
);
create table DirectoryEntity(
	`id` int primary key AUTO_INCREMENT,
    `dname` varchar(255) not null,
    `parent_id` int not null,
    `last_change_time` TIMESTAMP NOT NULL default current_timestamp on update current_timestamp
);
create table FileEntity(
	`id` int primary key AUTO_INCREMENT,
    `MD5` varchar(255) not null,
    `fsize` int not null,
    `link_num` int not null,
    constraint FE_link_num check (`link_num` >=0 and `fsize` >=0),
    `next_index` int not null,
    `is_complete` boolean not null
);
create table FileFragmentMap(
	`id` int primary key AUTO_INCREMENT,
    `fid` int not null,
    constraint FFM_fid foreign key(`fid`) references `FileEntity`(`id`),
    `index` int not null,
    `fgid` int not null,
    constraint FFM_fgid foreign key(`fgid`) references `FileFragmentEntity`(`id`)
);
create table FileDirectoryMap(
	`id` INT primary key AUTO_INCREMENT,
	`fid` INT not null,
    constraint FDM_fid foreign key(`fid`) references `FileEntity`(`id`),
	`did` INT not null,
    constraint FDM_did foreign key(`did`) references `DirectoryEntity`(`id`),
	`fname` VARCHAR(255) NOT NULL,
	`last_change_time` TIMESTAMP NOT NULL default current_timestamp on update current_timestamp
);
create table UserEntity(
	`id` INT primary key AUTO_INCREMENT,
	`user_name` VARCHAR(255) UNIQUE NOT NULL,
	`password_hash` VARCHAR(255) NOT NULL,
	`root_dir_id` INT NOT NULL,
    constraint UE_root_dir_id foreign key(`root_dir_id`) references `DirectoryEntity`(`id`)
);
create table UserLogin(
    `id` INT primary key AUTO_INCREMENT,
    `user_id` INT NOT NULL,
    constraint LUSER foreign key(`user_id`) references UserEntity(`id`),
    `login_ip` varchar(100) NOT NULL,
    `login_time` TIMESTAMP NOT NULL default current_timestamp on update current_timestamp
);
set global log_bin_trust_function_creators = 1;
drop function if exists get_root;
delimiter $$
create function get_root(did INT)
	returns int
    begin
    declare child int;
    declare parent int;
    set @child=0,@parent=did;
    while (@child!=@parent) do
		select id,parent_id into @child,@parent from DirectoryEntity where id=@parent;
	end while;
    return @parent;
    end $$

insert into DirectoryEntity(id,dname,parent_id) value (0,"default",0);