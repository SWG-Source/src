DECLARE
   CURSOR c1 IS
     select po.object_id, p.create_time, trunc(p.create_time) - to_date('31-DEC-2000', 'DD-MON-YYYY') right_born_date, po.born_date /*+ ORDERED */
     from player_objects po, objects o2, objects o, players p
     where p.character_object = o.object_id
           and o.object_id = o2.contained_by
	   and o2.object_id = po.object_id
	   and (po.born_date is null or po.born_date <> trunc(p.create_time) - to_date('31-DEC-2000', 'DD-MON-YYYY'));
BEGIN
   FOR c1rec IN c1 LOOP
      update player_objects
      set born_date = c1rec.right_born_date
      where object_id = c1rec.object_id;
      commit;
   END LOOP;
   COMMIT;
END;
/
