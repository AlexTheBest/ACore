delete from gameobject where id IN (select entry from gameobject_template where type=33);
