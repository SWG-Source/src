-- set all banks to load their contents normally instead of on-demand
UPDATE objects
SET load_contents = 'Y'
WHERE object_template_id = -172438875; -- object/tangible/bank/character_bank.iff

-- set load_with of items inside banks to their bank's contained_by (i.e. the character) 
UPDATE objects
SET load_with = (SELECT o1.load_with FROM objects o1 WHERE o1.object_id = objects.contained_by)
WHERE (SELECT o1.object_template_id FROM objects o1 WHERE o1.object_id = objects.contained_by) = -172438875; -- object/tangible/bank/character_bank.iff

