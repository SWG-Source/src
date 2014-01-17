delete from account_reward_items
where station_id in (select station_id from swg_characters
where cluster_id in (40,41));