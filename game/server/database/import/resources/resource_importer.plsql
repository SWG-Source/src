-- PL/SQL Package for importing resource data

create or replace package resource_importer
as
	procedure import_resources;
	procedure clear_import_tables;
end;
/

create or replace package body resource_importer
as
	procedure import_resources
	as
	begin
		delete from res_imp_oid_map;

		insert into res_imp_oid_map (object_id, reference_id, new)
		select rco.object_id, import_reference_id, 0
		from resource_class_objects rco, objects o
		where rco.object_id = o.object_id
		and o.deleted = 0;

		insert into res_imp_oid_map (object_id, reference_id, new)
		select objectidmanager.get_single_id, id, 1
		from resource_import
		where not exists (
			select * from res_imp_oid_map
			where res_imp_oid_map.reference_id = resource_import.id);

		insert into universe_objects (object_id)
		select object_id
		from res_imp_oid_map
		where new=1;

		insert into objects (object_id, x, y, z, quaternion_w, quaternion_x, quaternion_y, quaternion_z, node_x, node_y, node_z,
			object_template, type_id, scene_id, deleted,object_name,contained_by,slot_arrangement)
		select object_id, 0,0,0, 0,0,0,0, 0,0,0,
			'object/resource_class/resource_class.iff',1380270927,'universe',0,ri.name,0,-1
		from res_imp_oid_map m, resource_import ri
		where m.reference_id = ri.id
		and new=1;

		delete from resource_class_objects
		where object_id in (select object_id from res_imp_oid_map);

		insert into resource_class_objects (object_id,resource_class_name,min_types,max_types,parent_class,import_reference_id)
		select m.object_id,i.name,i.min_types,i.max_types,nvl(p.object_id,0),i.id
		from res_imp_oid_map m, resource_import i, res_imp_oid_map p
		where m.reference_id = i.id
		and p.reference_id (+) = i.parent_id;

		delete from object_variables
		where object_id in (select object_id from res_imp_oid_map);

		insert into object_variables (object_id, name, type, value)
		select m.object_id, i.name, i.type, i.value
		from res_imp_oid_map m, res_imp_object_variables i
		where m.reference_id = i.id;
	end;

	procedure clear_import_tables
	as
	begin
		delete from res_imp_oid_map;
		delete from resource_import;
		delete from res_imp_object_variables;
	end;

end;
/
