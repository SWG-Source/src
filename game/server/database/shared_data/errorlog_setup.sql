delete from errorlog_level_desc;

insert into errorlog_level_desc (levelid,descrip) values (0,'DB Error logging disabled.');
insert into errorlog_level_desc (levelid,descrip) values (1,'Basic DB Error logging enabled.');
insert into errorlog_level_desc (levelid,descrip) values (2,'Detailed DB Error logging enabled.');

delete from errorlog_level;

insert into errorlog_level (levelid) values (2);
