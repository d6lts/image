<?php
// $Id$
/**
 * @file
 *
 * Theme preprocessors for image plugins.
 */

/**
 * Preprocessor for the gallery style plugin.
 */
function template_preprocess_image_view_image_gallery(&$vars) {
  drupal_add_css(drupal_get_path('module', 'image') .'/image.css');
}
