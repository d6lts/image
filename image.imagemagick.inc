<?php
// $Id$

/**
 * Return information about the imagemagick toolkit
 */
function image_imagemagick_info() {
  return array('name' => 'imagemagick', 'title' => 'ImageMagick Toolkit.');
}

/**
 * Validate and return toolkit specific settings
 */
function image_imagemagick_settings() {
  $form['#after_build'] = array('_image_imagemagick_build_version');
  $form['image_imagemagick_convert'] = array(
    '#type' => 'textfield',
    '#title' => t('Path to the "convert" binary'),
    '#default_value' => variable_get('image_imagemagick_convert', '/usr/bin/convert'),
    '#required' => TRUE,
  );
  return $form;
}

function _image_imagemagick_build_version($form, $form_element) {
  $valid = _image_imagemagick_check_path($form_element['image_imagemagick_convert'], 'image_imagemagick_convert');
  if ($valid) {
    _image_imagemagick_convert_exec('-version', $output, $errors);
    $form['image_imagemagick_version'] = array(
      '#type' => 'item',
      '#title' => t('Version'),
      '#value' => '<pre>'. check_plain($output) .'</pre>',
    );
  }
  return $form;
}

function _image_imagemagick_check_path($path, $attach_error_to = FALSE) {
  if (is_file($path)) {
    return TRUE;
  }
  if ($attach_error_to) {
    if (ini_get('open_basedir')) {
      form_set_error($attach_error_to, t("No file %file could be found. PHP's <a href='@open-basedir'>open_basedir</a> security resriction may be interfearing with the attempts to locate it.", array('%file' => $path, '@open-basedir' => url('http://us2.php.net/features.safe-mode') )));
    }
    else {
      form_set_error($attach_error_to, t('The specified ImageMagic path %file does not exist.', array('%file' => $path)));
    }
  }
  return FALSE;
}

/**
 * Resize an image to the given width and height
 */
function image_imagemagick_resize($source, $dest, $width, $height) {
  $filter = ' -scale '. $width .'x'. $height .'! -filter QUADRATIC';
  return _image_imagemagick_convert($source, $dest, $filter);
}

/**
 * Rotate an image
 */
function image_imagemagick_rotate($source, $dest, $degrees) {
  $filter = ' -rotate '. escapeshellarg($degrees) .' -background #000000';
  return _image_imagemagick_convert($source, $dest, $filter);
}

/**
 * Crop an image to the specified dimensions
 */
function image_imagemagick_crop($source, $dest, $x, $y, $width, $height) {
  $filter = ' -crop '. $width .'x'. $height .'+'. $x .'+'. $y;
  return _image_imagemagick_convert($source, $dest, $filter);
}

/**
 * Calls the convert executable with the specified filter
 */
function _image_imagemagick_convert($source, $dest, $filter) {
  $command = implode(' ', array(
    preg_replace("/[^A-Za-z0-9\!\.\-\+\040]/", '', $filter),
    escapeshellarg($source),
    escapeshellarg($dest),
  ));

  if (0 != _image_imagemagick_convert_exec($command, $output, $errors)) {
    return FALSE;
  }
  return file_exists($dest);
}

function _image_imagemagick_convert_exec($command_args, &$output, &$errors) {
  $convert_path = variable_get('image_imagemagick_convert', '/usr/bin/convert');
  if (!_image_imagemagick_check_path($convert_path)) {
    drupal_set_message(t("ImageMagick could not be found. The admin will need to set the path on the <a href='@image-toolkit-settings'>image toolkit page</a>.", array('@image-toolkit-settings' => url('admin/settings/image-toolkit'))), 'error');
    return FALSE;
  }

  if (strstr($_SERVER['SERVER_SOFTWARE'], 'Win32') || strstr($_SERVER['SERVER_SOFTWARE'], 'IIS')) {
    // use window's start command to avoid the "black window" from showing up:
    // http://us3.php.net/manual/en/function.exec.php#56599
    // use /D to run the command from PHP's current working directory so the
    // file paths don't have to be absolute.
    $convert_path = 'start "window title" /D'. getcwd() .' /b '. escapeshellarg($convert_path);
  }

  $descriptors = array(
    0 => array('pipe', 'r'), // stdin
    1 => array('pipe', 'w'), // stdout
    2 => array('pipe', 'w')  // stderr
  );
  if ($h = proc_open($convert_path .' '. $command_args, $descriptors, $pipes)) {
    $output = '';
    while (!feof($pipes[1])) {
      $output .= fgets($pipes[1]);
    }

    $errors = '';
    while (!feof($pipes[2])) {
      $errors .= fgets($pipes[2]);
    }

    #drupal_set_message(t("ImageMagick command: %command", array('%command' => $convert_path .' '. $command_args)));
    #drupal_set_message(t("ImageMagick output: %output", array('%output' => $output)));
    if ($errors) {
      drupal_set_message(t("ImageMagick reported an error: %error", array('%error' => $errors)), 'error');
    }

    fclose($pipes[0]);
    fclose($pipes[1]);
    fclose($pipes[2]);
    return proc_close($h);
  }
  return FALSE;
}

