update schema_name.tabs set parent = 0 where id in (select id from schema_name.tabs where parent not in (select id from schema_name.tabs) and parent != 0);
