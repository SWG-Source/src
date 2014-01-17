create table object_templates  -- NO_IMPORT
(
	id number,
	name varchar2(255),
	constraint pk_object_templates primary key (id) using index tablespace indexes
);

create index object_templates_name_idx on object_templates (name) tablespace indexes;
grant select on object_templates to public;

update version_number set version_number=96, min_version_number=96;
