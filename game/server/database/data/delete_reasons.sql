delete from delete_reasons;

insert into delete_reasons values (0,'NotDeleted','not deleted');	
insert into delete_reasons values (1,'Unknown','unknown');
insert into delete_reasons values (2,'Decay','deleted by decay');
insert into delete_reasons values (3,'God','deleted by a god client or console command');
insert into delete_reasons values (4,'Player','deleted by a player action');
insert into delete_reasons values (5,'Script','deleted by a script');
insert into delete_reasons values (6,'CharacterDeleted','deleted because the player deleted the character');
insert into delete_reasons values (7,'ContainerDeleted','deleted because the container was deleted');
insert into delete_reasons values (8,'Publish','deleted by the publish process');
insert into delete_reasons values (9,'BadContainerTransfer','deleted because the object could not be placed in a particular container');
insert into delete_reasons values (10,'Consumed','deleted because the object was used up or consumed by the game');
insert into delete_reasons values (11,'SetupFailed','deleted because an initialization step related to the object failed');
insert into delete_reasons values (12,'Replaced','deleted because the object was replaced by another with newer data');
insert into delete_reasons values (13,'House','deleted house from script');

