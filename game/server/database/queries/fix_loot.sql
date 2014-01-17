update objects set
  objvar_0_value = (case when objvar_0_name like 'dot.loot__.uses' and objvar_0_value = '-1' then '3000' else objvar_0_value end),
  objvar_1_value = (case when objvar_1_name like 'dot.loot__.uses' and objvar_1_value = '-1' then '3000' else objvar_1_value end),
  objvar_2_value = (case when objvar_2_name like 'dot.loot__.uses' and objvar_2_value = '-1' then '3000' else objvar_2_value end),
  objvar_3_value = (case when objvar_3_name like 'dot.loot__.uses' and objvar_3_value = '-1' then '3000' else objvar_3_value end),
  objvar_4_value = (case when objvar_4_name like 'dot.loot__.uses' and objvar_4_value = '-1' then '3000' else objvar_4_value end),
  objvar_5_value = (case when objvar_5_name like 'dot.loot__.uses' and objvar_5_value = '-1' then '3000' else objvar_5_value end),
  objvar_6_value = (case when objvar_6_name like 'dot.loot__.uses' and objvar_6_value = '-1' then '3000' else objvar_6_value end),
  objvar_7_value = (case when objvar_7_name like 'dot.loot__.uses' and objvar_7_value = '-1' then '3000' else objvar_7_value end),
  objvar_8_value = (case when objvar_8_name like 'dot.loot__.uses' and objvar_8_value = '-1' then '3000' else objvar_8_value end),
  objvar_9_value = (case when objvar_9_name like 'dot.loot__.uses' and objvar_9_value = '-1' then '3000' else objvar_9_value end),
  objvar_10_value = (case when objvar_10_name like 'dot.loot__.uses' and objvar_10_value = '-1' then '3000' else objvar_10_value end),
  objvar_11_value = (case when objvar_11_name like 'dot.loot__.uses' and objvar_11_value = '-1' then '3000' else objvar_11_value end),
  objvar_12_value = (case when objvar_12_name like 'dot.loot__.uses' and objvar_12_value = '-1' then '3000' else objvar_12_value end),
  objvar_13_value = (case when objvar_13_name like 'dot.loot__.uses' and objvar_13_value = '-1' then '3000' else objvar_13_value end),
  objvar_14_value = (case when objvar_14_name like 'dot.loot__.uses' and objvar_14_value = '-1' then '3000' else objvar_14_value end),
  objvar_15_value = (case when objvar_15_name like 'dot.loot__.uses' and objvar_15_value = '-1' then '3000' else objvar_15_value end),
  objvar_16_value = (case when objvar_16_name like 'dot.loot__.uses' and objvar_16_value = '-1' then '3000' else objvar_16_value end),
  objvar_17_value = (case when objvar_17_name like 'dot.loot__.uses' and objvar_17_value = '-1' then '3000' else objvar_17_value end),
  objvar_18_value = (case when objvar_18_name like 'dot.loot__.uses' and objvar_18_value = '-1' then '3000' else objvar_18_value end),
  objvar_19_value = (case when objvar_19_name like 'dot.loot__.uses' and objvar_19_value = '-1' then '3000' else objvar_19_value end)
where (objvar_0_name like 'dot.loot__.uses' and objvar_0_value = '-1') or
      (objvar_1_name like 'dot.loot__.uses' and objvar_1_value = '-1') or
      (objvar_2_name like 'dot.loot__.uses' and objvar_2_value = '-1') or
      (objvar_3_name like 'dot.loot__.uses' and objvar_3_value = '-1') or
      (objvar_4_name like 'dot.loot__.uses' and objvar_4_value = '-1') or
      (objvar_5_name like 'dot.loot__.uses' and objvar_5_value = '-1') or
      (objvar_6_name like 'dot.loot__.uses' and objvar_6_value = '-1') or
      (objvar_7_name like 'dot.loot__.uses' and objvar_7_value = '-1') or
      (objvar_8_name like 'dot.loot__.uses' and objvar_8_value = '-1') or
      (objvar_9_name like 'dot.loot__.uses' and objvar_9_value = '-1') or
      (objvar_10_name like 'dot.loot__.uses' and objvar_10_value = '-1') or
      (objvar_11_name like 'dot.loot__.uses' and objvar_11_value = '-1') or
      (objvar_12_name like 'dot.loot__.uses' and objvar_12_value = '-1') or
      (objvar_13_name like 'dot.loot__.uses' and objvar_13_value = '-1') or
      (objvar_14_name like 'dot.loot__.uses' and objvar_14_value = '-1') or
      (objvar_15_name like 'dot.loot__.uses' and objvar_15_value = '-1') or
      (objvar_16_name like 'dot.loot__.uses' and objvar_16_value = '-1') or
      (objvar_17_name like 'dot.loot__.uses' and objvar_17_value = '-1') or
      (objvar_18_name like 'dot.loot__.uses' and objvar_18_value = '-1') or
      (objvar_19_name like 'dot.loot__.uses' and objvar_19_value = '-1');

update object_variables set
  value = '3000'
where name_id in (select id from object_variable_names where name like 'dot.loot__.uses') and
      value = '-1';

commit;
