UPDATE:

From 4.3.0RC version and up, for PostgreSQL users, it is no longer necessary to replace all occurrences of RAND() with RANDOM() in image.module.
Thanks to Adrian, update.php includes now a function to create a RAND() function within PostgreSQL, which just wraps the existing RANDOM(). See update_65().

For 4.2.0 version, image module has been improved to use the correct function in queries, RAND() or RANDOM(), based on the database type.
