create or replace package body undelete_methods is

  FUNCTION restore_object
    (object_id_in     IN objects.object_id%TYPE)
  RETURN NUMBER IS
  BEGIN 
    --update an object and its children
    --continue down tree with any object 
    --whose contained_by is a valid object id.  
    UPDATE objects
    SET deleted = 0,
        deleted_date = NULL
    WHERE object_id = object_id_in;
       
    COMMIT;                    
    RETURN 0;
  
    EXCEPTION 
      WHEN OTHERS THEN
      ROLLBACK;
      RETURN SQLCODE;
  END restore_object;
 
end undelete_methods;
/
