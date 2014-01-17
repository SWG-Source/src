alter table resource_pool_objects drop column amount_remaining;
alter table resource_pool_objects drop column last_simulated_timestamp;
alter table resource_pool_objects drop column drain_rate;
alter table resource_pool_objects drop column tick_count;
alter table resource_pool_objects drop column regeneration_rate;
alter table resource_pool_objects drop column max_pool_size;

update version_number set version_number=95, min_version_number=95;
