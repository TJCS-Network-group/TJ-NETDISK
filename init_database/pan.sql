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

insert into DirectoryEntity(id,dname,parent_id) value (1, 'root', 1);
insert into DirectoryEntity(id,dname,parent_id) value (2, 'root', 2);
insert into DirectoryEntity(id,dname,parent_id) value (3, 'root', 3);
insert into DirectoryEntity(dname,parent_id) value ('book', 1);
insert into DirectoryEntity(dname,parent_id) value ('test', 4);
insert into DirectoryEntity(dname,parent_id) value ('test', 2);
insert into UserEntity(user_name,password_hash,root_dir_id) value ('test', '098f6bcd4621d373cade4e832627b4f6', 3);
insert into UserEntity(user_name,password_hash,root_dir_id) value ('test1', '5a105e8b9d40e1329780d62ea2265d8a', 2);
insert into UserEntity(user_name,password_hash,root_dir_id) value ('test2', 'ad0234829205b9033196ba818f7a872b', 1);
insert into FileEntity(MD5,fsize,link_num,next_index,is_complete) value ('74b2aa5e188309873c992c13b808860d', 56619636, 2, -1, 1);
insert into FileFragmentEntity(MD5,fgsize,link_num) value ('8bea2fce3785775f2dfb2a6401e8105c', 4194304, 1);
insert into FileFragmentMap(fid,`index`,fgid) value (1, 0, 1);
insert into FileFragmentEntity(MD5,fgsize,link_num) value ('6187fc78f73e5f00cdf799eb36327d0f', 4194304, 1);
insert into FileFragmentMap(fid,`index`,fgid) value (1, 1, 2);
insert into FileFragmentEntity(MD5,fgsize,link_num) value ('50f45500ab89239cd4affeb8813c8acd', 4194304, 1);
insert into FileFragmentMap(fid,`index`,fgid) value (1, 2, 3);
insert into FileFragmentEntity(MD5,fgsize,link_num) value ('b6167926c5265bd6fa43aa4531211cb0', 4194304, 1);
insert into FileFragmentMap(fid,`index`,fgid) value (1, 3, 4);
insert into FileFragmentEntity(MD5,fgsize,link_num) value ('aeb4dc6274d783aa224a791fb375c7e0', 4194304, 1);
insert into FileFragmentMap(fid,`index`,fgid) value (1, 4, 5);
insert into FileFragmentEntity(MD5,fgsize,link_num) value ('35fcb3fb4ba2563078d4d5aeae51b859', 4194304, 1);
insert into FileFragmentMap(fid,`index`,fgid) value (1, 5, 6);
insert into FileFragmentEntity(MD5,fgsize,link_num) value ('2d49bdbe6a661e49c969251c06eca916', 4194304, 1);
insert into FileFragmentMap(fid,`index`,fgid) value (1, 6, 7);
insert into FileFragmentEntity(MD5,fgsize,link_num) value ('f7b776b92af76b21916bffe1f006695a', 4194304, 1);
insert into FileFragmentMap(fid,`index`,fgid) value (1, 7, 8);
insert into FileFragmentEntity(MD5,fgsize,link_num) value ('aeade4ceef5bf491f316ecd98f78e930', 4194304, 1);
insert into FileFragmentMap(fid,`index`,fgid) value (1, 8, 9);
insert into FileFragmentEntity(MD5,fgsize,link_num) value ('d11cc1229712568da339f64ad376e9e1', 4194304, 1);
insert into FileFragmentMap(fid,`index`,fgid) value (1, 9, 10);
insert into FileFragmentEntity(MD5,fgsize,link_num) value ('713579df117913a3c8864c5f98d83c08', 4194304, 1);
insert into FileFragmentMap(fid,`index`,fgid) value (1, 10, 11);
insert into FileFragmentEntity(MD5,fgsize,link_num) value ('097e013f020ffbba20cbad9e9ccb0d46', 4194304, 1);
insert into FileFragmentMap(fid,`index`,fgid) value (1, 11, 12);
insert into FileFragmentEntity(MD5,fgsize,link_num) value ('029cd2000b99973f8fe3511a2a5499ed', 4194304, 1);
insert into FileFragmentMap(fid,`index`,fgid) value (1, 12, 13);
insert into FileFragmentEntity(MD5,fgsize,link_num) value ('762ce70edc9f8cbf12437c76ca77c9d8', 2093684, 1);
insert into FileFragmentMap(fid,`index`,fgid) value (1, 13, 14);
insert into FileDirectoryMap(fid,did,fname) value (1, 6, 'test.pdf');
insert into FileDirectoryMap(fid,did,fname) value (1, 5, 'book.pdf');