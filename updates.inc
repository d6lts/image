<?php
/* $Id$ */

// Define the various updates in an array("date : comment" => "function");
$sql_updates = array(
  "2004-07-28: first update since Drupal 4.4.0 release" => "update_1"
);

function update_1() {
  $ret = array();
  if ($GLOBALS["db_type"] == "pgsql") {
    $ret[] = update_sql("ALTER TABLE {image} ADD image_list(carchar(255))");
  } else {
    $ret[] = update_sql("ALTER TABLE {image} ADD image_list LONGTEXT");
  }
  return $ret;
}

function update_sql($sql) {
  $edit = $_POST["edit"];
  $result = db_query($sql);
  if ($result) {
    return array('1', nl2br(htmlentities($sql)) ." ", "<div style=\"color: green;\">OK</div>\n");
  }
  else {
    return array('0', nl2br(htmlentities($sql)) ." ", "<div style=\"color: red;\">FAILED</div>\n");
  }
}

?>
