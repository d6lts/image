<?php
// $Id$
/*
** USAGE:
**
** - Point your browser to "http://www.site.com/update-image.php" and follow
**   the instructions.
**
** - If you are not logged in as administrator, you will need to modify the
**   statement below. Change the 1 into a 0 to disable the access check.
**   After finishing the upgrade, open this file and change the 0 back into
**   a 1!
*/

// Disable access checking?

$access_check = 1;

if (!ini_get("safe_mode")) {
  set_time_limit(180);
}

include_once "modules/image/updates.inc";

function update_data($start) {
  global $sql_updates;
  $sql_updates = array_slice($sql_updates, ($start-- ? $start : 0));
  foreach ($sql_updates as $date => $func) {
    print "<strong>$date</strong><br />\n<pre>\n";
    $ret = $func();
    foreach ($ret as $return) {
      print $return[1];
      print $return[2];
    }
    variable_set("update_image_start", $date);
    print "</pre>\n";
  }
}

function update_page_header($title) {
  $output = "<html><head><title>$title</title>";
  $output .= <<<EOF
      <link rel="stylesheet" type="text/css" media="print" href="misc/print.css" />
      <style type="text/css" title="layout" media="Screen">
        @import url("misc/drupal.css");
      </style>
EOF;
  $output .= "</head><body>";
  $output .= "<div id=\"logo\"><a href=\"http://drupal.org/\"><img src=\"misc/druplicon-small.png\" alt=\"Druplicon - Drupal logo\" title=\"Druplicon - Drupal logo\" /></a></div>";
  $output .= "<div id=\"update\"><h1>$title</h1>";
  return $output;
}

function update_page_footer() {
  return "</div></body></html>";
}

function update_page() {
  global $user, $sql_updates;

  $edit = $_POST["edit"];

  switch ($_POST["op"]) {
    case "Update":
      // make sure we have updates to run.
      print update_page_header("Drupal image.module update");
      $links[] = "<a href=\"index.php\">main page</a>";
      $links[] = "<a href=\"index.php?q=admin\">administration pages</a>";
      print theme("item_list", $links);
        // NOTE: we can't use l() here because the URL would point to 'update-image.php?q=admin'.
      if ($edit["start"] == -1) {
        print "No updates to perform.";
      }
      else {
        update_data($edit["start"]);
      }
      print "<br />Updates were attempted. If you see no failures above, you may proceed happily to the <a href=\"index.php?q=admin\">administration pages</a>.";
      print " Otherwise, you may need to update your database manually.";
      print update_page_footer();
      break;
    default:
      $start = variable_get("update_image_start", 0);
      $dates[] = "All";
      $i = 1;
      foreach ($sql_updates as $date => $sql) {
        $dates[$i++] = $date;
        if ($date == $start) {
          $selected = $i;
        }
      }
      $dates[$i] = "No updates available";

      // make update form and output it.
      $form .= form_select("Perform updates from", "start", (isset($selected) ? $selected : -1), $dates, "This defaults to the first available update since the last update you peformed.");
      $form .= form_submit("Update");
      print update_page_header("Drupal image.module update");
      print form($form);
      print update_page_footer();
      break;
  }
}

function update_info() {
  print update_page_header("Drupal image.module update");
  print "<ol>\n";
  print "<li>Use this script to <strong>upgrade an existing Drupal installation that is using image.module</strong>.  You don't need this script when installing Drupal from scratch.</li>";
  print "<li>Before doing anything, backup your database. This process will change your database and its values, and some things might get lost.</li>\n";
  print "<li>Update your Drupal sources, check the notes below and <a href=\"update-image.php?op=update\">run the database upgrade script</a>.  Don't upgrade your database twice as it may cause problems.</p></li>\n";
  print "<li>Go through the various administration pages to change the existing and new settings to your liking.</li>\n";
  print "</ol>";
  print "</ol>";
  print update_page_footer();
}

if (isset($_GET["op"])) {
  include_once "includes/bootstrap.inc";
  include_once "includes/common.inc";

  // Access check:
  if (($access_check == 0) || ($user->uid == 1)) {
    update_page();
  }
  else {
    print update_page_header("Access denied");
    print "Access denied.  You are not authorized to access to this page.  Please log in as the user with user ID #1. If you cannot log-in, you will have to edit <code>update-image.php</code> to by-pass this access check; in that case, open <code>update-image.php</code> in a text editor and follow the instructions at the top.";
    print update_page_footer();
  }
}
else {
  update_info();
}
?>
