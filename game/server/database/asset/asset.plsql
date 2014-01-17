PROMPT CREATE OR REPLACE PACKAGE asset.asset_exporter

CREATE OR REPLACE PACKAGE asset.ASSET_EXPORTER
as
	function update_shader (p_pathname varchar2, p_filename varchar2, p_effect varchar2)
	        return number;
        function update_mesh (
		p_pathname varchar2,
		p_filename varchar2,
		p_type varchar2,
		p_num_polygons number,
		p_num_verts number,
		p_num_uv_sets number,
		p_extent_center_x float,
		p_extent_center_y float,
		p_extent_center_z float,
		p_extent_radius float,
		p_extent_box_min_x float,
		p_extent_box_min_y float,
		p_extent_box_min_z float,
		p_extent_box_max_x float,
		p_extent_box_max_y float,
		p_extent_box_max_z float,
		p_num_hardpoints number)
        	return number;
        function update_texture (p_pathname varchar2, p_filename varchar2, p_texture_size number, p_depth number, p_num_mips number)
	        return number;
        function update_lod (p_pathname varchar2, p_filename varchar2)
	        return number;
        function update_component (p_pathname varchar2, p_filename varchar2)
	        return number;
        function update_sprite (p_pathname varchar2, p_filename varchar2, p_radius float, p_center_x float,
		p_center_y float, p_center_z float)
	        return number;
        function update_skeleton (p_pathname varchar2, p_filename varchar2, p_num_bones number)
	        return number;
        function update_animation (p_pathname varchar2, p_filename varchar2, p_num_frames number, p_type varchar2, p_gender number)
	        return number;
        function find_art_item(p_pathname varchar2, p_filename varchar2, p_type number)
	        return number;

	procedure wipe_tree(p_appearance_id number);
	procedure add_tree_node(p_appearance_id number, p_item_id number, p_entry_index number, p_exit_index number, p_parent_entry number);

	procedure set_appearance_property(p_appearance_id number, p_name varchar2, p_value varchar2);
	procedure update_appearance(p_appearance_id number, p_author varchar2);
	procedure update_appearance2(p_appearance_id number, p_author varchar2, p_status number);
end;

/

PROMPT CREATE OR REPLACE PACKAGE BODY asset.asset_exporter

CREATE OR REPLACE PACKAGE BODY asset.ASSET_EXPORTER as

	function update_shader (p_pathname varchar2, p_filename varchar2, p_effect varchar2)
	return number
	as
		item_id number;
	begin
		item_id := find_art_item(p_pathname, p_filename,1);

		update shaders
		set effect=p_effect
		where id=item_id;

		if sql%rowcount = 0 then
			insert into shaders (id,effect)
			values (item_id,p_effect);
		end if;

		return item_id;
	end;

	function update_mesh (
		p_pathname varchar2,
		p_filename varchar2,
		p_type varchar2,
		p_num_polygons number,
		p_num_verts number,
		p_num_uv_sets number,
		p_extent_center_x float,
		p_extent_center_y float,
		p_extent_center_z float,
		p_extent_radius float,
		p_extent_box_min_x float,
		p_extent_box_min_y float,
		p_extent_box_min_z float,
		p_extent_box_max_x float,
		p_extent_box_max_y float,
		p_extent_box_max_z float,
		p_num_hardpoints number)
	return number
	as
		item_id number;
	begin
		item_id := find_art_item(p_pathname, p_filename,2);

		update meshes
		set
			type=p_type,
			num_verts=p_num_verts,
			num_uv_sets=p_num_uv_sets,
			extent_center_x=p_extent_center_x,
			extent_center_y=p_extent_center_y,
			extent_center_z=p_extent_center_z,
			extent_radius=p_extent_radius,
			extent_box_min_x=p_extent_box_min_x,
			extent_box_min_y=p_extent_box_min_y,
			extent_box_min_z=p_extent_box_min_z,
			extent_box_max_x=p_extent_box_max_x,
			extent_box_max_y=p_extent_box_max_y,
			extent_box_max_z=p_extent_box_max_z
		where id=item_id;

		if sql%rowcount = 0 then
			insert into meshes (id,type,num_polygons,num_verts,num_uv_sets,extent_center_x,extent_center_y,
				extent_center_z,extent_radius,extent_box_min_x,extent_box_min_y,extent_box_min_z,
				extent_box_max_x,extent_box_max_y,extent_box_max_z)
			values (item_id,p_type,p_num_polygons,p_num_verts,p_num_uv_sets,p_extent_center_x,p_extent_center_y,
				p_extent_center_z,p_extent_radius,p_extent_box_min_x,p_extent_box_min_y,p_extent_box_min_z,
				p_extent_box_max_x,p_extent_box_max_y,p_extent_box_max_z);
		end if;

		return item_id;
	end;

	function update_texture (p_pathname varchar2, p_filename varchar2, p_texture_size number, p_depth number, p_num_mips number)
	return number
	as
		item_id number;
	begin
		item_id := find_art_item(p_pathname, p_filename,3);

		update textures
		set texture_size=p_texture_size, depth=p_depth, num_mips=p_num_mips
		where id=item_id;

		if sql%rowcount = 0 then
			insert into textures (id,texture_size,depth,num_mips)
			values (item_id,p_texture_size,p_depth,p_num_mips);
		end if;

		return item_id;
	end;

	function update_lod (p_pathname varchar2, p_filename varchar2)
	return number
	as
		item_id number;
	begin
		item_id := find_art_item(p_pathname, p_filename,4);

		insert into lods (id)
		select item_id
		from dual
		where not exists (
			select * from lods
			where id=item_id);

		return item_id;
	end;

	function update_component (p_pathname varchar2, p_filename varchar2)
	return number
	as
		item_id number;
	begin
		item_id := find_art_item(p_pathname, p_filename,5);

		insert into components (id)
		select item_id
		from dual
		where not exists (
			select * from components
			where id=item_id);

		return item_id;
	end;

	function update_sprite (p_pathname varchar2, p_filename varchar2, p_radius float, p_center_x float,
		p_center_y float, p_center_z float)
	return number
	as
		item_id number;
	begin
		item_id := find_art_item(p_pathname, p_filename,6);

		update sprites
		set radius=p_radius, center_x=p_center_x, center_y=p_center_y, center_z=p_center_z
		where id=item_id;

		if sql%rowcount = 0 then
			insert into sprites (id,radius,center_x,center_y,center_z)
			values (item_id, p_radius, p_center_x, p_center_y, p_center_z);
		end if;

		return item_id;
	end;

	function update_skeleton (p_pathname varchar2, p_filename varchar2, p_num_bones number)
	return number
	as
		item_id number;
	begin
		item_id := find_art_item(p_pathname, p_filename,7);

		update skeletons
		set num_bones=p_num_bones
		where id=item_id;

		if sql%rowcount = 0 then
			insert into skeletons (id,num_bones)
			values (item_id, p_num_bones);
		end if;

		return item_id;
	end;

	function update_animation (p_pathname varchar2, p_filename varchar2, p_num_frames number, p_type varchar2, p_gender number)
	return number
	as
		item_id number;
	begin
		item_id := find_art_item(p_pathname, p_filename,8);

		update animations
		set num_frames=p_num_frames, type=p_type, gender=p_gender
		where id=item_id;

		if sql%rowcount = 0 then
			insert into animations (id, num_frames, type, gender)
			values (item_id, p_num_frames, p_type, p_gender);
		end if;

		return item_id;
	end;

	function find_art_item(p_pathname varchar2, p_filename varchar2, p_type number)
	return number
	as
		item_id number;
	begin
		select id
		into item_id
		from art_items
		where pathname = p_pathname
		and filename = p_filename;

		return item_id;

		exception
		when NO_DATA_FOUND then
			select art_item_ids.nextval
			into item_id
			from dual;

			insert into art_items (id, pathname, filename, type_id)
			values (item_id, p_pathname, p_filename, p_type);

			return item_id;
	end;

	procedure wipe_tree(p_appearance_id number)
	as
	begin
		delete from art_asset_tree where appearance_id=p_appearance_id;
	end;


	procedure add_tree_node(p_appearance_id number, p_item_id number, p_entry_index number, p_exit_index number, p_parent_entry number)
	as
	begin
		insert into art_asset_tree (appearance_id,parent_entry,entry_index,exit_index,item_id)
		values (p_appearance_id, p_parent_entry, p_entry_index, p_exit_index, p_item_id);
	end;

	procedure set_appearance_property(p_appearance_id number, p_name varchar2, p_value varchar2)
	as
	begin
		update appearance_properties
		set property_value=p_value
		where appearance_id = p_appearance_id
		and property_name = p_name;

		if sql%rowcount = 0 then
			insert into appearance_properties (appearance_id, property_name, property_value)
			values (p_appearance_id, p_name, p_value);
		end if;
	end;

	procedure update_appearance(p_appearance_id number, p_author varchar2)
	as
	begin
		update appearances
		set author=p_author,
		exported_date = sysdate
		where id = p_appearance_id;
	end;

	procedure update_appearance2(p_appearance_id number, p_author varchar2, p_status number)
	as
	begin
		update appearances
		set author=p_author,
		status_id=p_status,
		exported_date = sysdate
		where id = p_appearance_id;
	end;

end;

/

