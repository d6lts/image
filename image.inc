<?php
// $Id$

/**
 * @file
 * Common functions to manipulate and get information about files.
 */

/**
 * @defgroup metadata
 *   Retrieve metadata about images.
 * @{
 */

/**
 * Returns information about the image.
 *
 * @param $image path to the image
 *
 * @return array containing information about the image
 *      'width': image's width in pixels
 *      'height': image's height in pixels
 *      'type': image type ('GIF', 'JPEG', etc., or 'UNKNOWN')
 *      'mime_type': image's MIME type ('image/jpeg', 'image/gif', etc.)
 *      'iptc_info': array containing IPTC information (see _image_iptc below)
 */
function image_get_info($image) {

  $details = array();
  $data = @getimagesize($image, $raw_iptc_info);

  if (is_array($data)){
    $types = array('1' => 'GIF', '2' => 'JPEG', '3' => 'PNG', '4' => 'SWF', '5' => 'PSD', '6' => 'BMP', '7' => 'TIFF', '8' => 'TIFF', '9' => 'JPC', '10' => 'JP2', '11' => 'JPX', '12' => 'JB2', '13' => 'SWC', '14' => 'IFF', '15' => 'WBMP', '16' => 'XBM');
    $type = array_key_exists($data[2], $types) ? $types[$data[2]] : 'UNKNOWN';
    $iptc_info = _image_iptc($raw_iptc_info);
    $details = array('width'     => $data[0],
                     'height'    => $data[1],
                     'type'      => $type,
                     'mime_type' => $data['mime'],
                     'iptc_info' => $iptc_info);
  }
  return $details;
}

/**
 * Internal function: Parse IPTC data for image. Called by image_get_info.
 *
 * @param &$info info array returned by PHP getimagesize function
 *
 * @return array containing IPTC information
 */
function _image_iptc(&$info) {

  // Credit to pkrohn@daemonize.com from php online manual for this.

  if (isset($info['APP13'])) {

    $iptc = iptcparse($info['APP13']);
    if (is_array($iptc)) {
      $result['caption'] = $iptc['2#120'][0];
      $result['graphic_name'] = $iptc['2#005'][0];
      $result['urgency'] = $iptc['2#010'][0];
      $result['category'] = $iptc['2#015'][0];
      // note that sometimes supp_categories contains multiple entries
      $result['supp_categories'] = $iptc['2#020'];
      $result['spec_instr'] = $iptc['2#040'][0];
      $result['creation_date'] = $iptc['2#055'][0];
      $result['photog'] = $iptc['2#080'][0];
      $result['credit_byline_title'] = $iptc['2#085'][0];
      $result['city'] = $iptc['2#090'][0];
      $result['state'] = $iptc['2#095'][0];
      $result['country'] = $iptc['2#101'][0];
      $result['otr'] = $iptc['2#103'][0];
      $result['headline'] = $iptc['2#105'][0];
      $result['source'] = $iptc['2#110'][0];
      $result['photo_source'] = $iptc['2#115'][0];
      $result['caption'] = $iptc['2#120'][0];
      $result['keywords'] = $iptc['2#025'];
      $result['caption_writer'] = $iptc['2#122'][0];
    }
  }
  return $result;
}

/**
 * Returns EXIF information from an image.
 *
 * @param $file path to image file
 *
 * @return array containing parsed EXIF information
 */
function image_get_exif($file) {

  // From gallery 1.2.5 (also under GPL, see http://cvs.sourceforge.net/viewcvs.py/gallery/gallery/LICENSE.txt?rev=1.2&view=log).

  $_exif_path = variable_get('image_jhead_path', '');

  if ($_exif_path) {

    $return = exec($_exif_path . ' ' . _image_escape_shell($file), $results);
    
    while (list($key,$value) = each($results)) {
      $explodeReturn = explode(':', $value, 2);
      $header = trim($explodeReturn[0]);
      if ($header != 'File name' && $header != 'File date' && $header != 'File size' && $header != 'Not JPEG') {
        $myExif[$header] = trim($explodeReturn[1]);
      }
    }

    return $myExif;
  }
}

/**
 * Check that image type is supported by current image library.
 * Currently GIF type can be a problem with GD as GIF image creation
 * has been removed from this library until patent expires worldwide.
 *
 * @param $type file type in question (see 'type' result from image_get_info)
 *
 * @return true if (possibly) supported; false if not
 */
function image_type_is_supported($type) {
  switch (variable_get('image_lib', 'imagemagick')) {
    case 'gd1':
    case 'gd2':
      $supported = in_array($type, _get_gd_image_types());
      break;
    case 'libmagick':
      $supported = true;
      break;
    case 'imlib2':
      $supported = true;
      break;
    case 'imagemagick':
      $supported = true;
      break;
    default:
      $supported = false;
  }
  return $supported;
}

/**
 * @}
 */

/**
 * @defgroup image_manipulation
 *  Abstraction layer for image manipulation functions.
 * @{
 */

/**
 * Open an image file for manipulation.
 *
 * @param $filename path to the image file
 *
 * @return $image object to be passed to succeeding image_ functions
 */
function image_open($filename) {
  $imagelib = variable_get('image_lib', 'imagemagick');
  $function = '_image_open_' . $imagelib;

  if (function_exists($function)) {
    return $function($filename);
  } else {
    return false;
  }
}

/**
 * Finish manipulating an image file.
 */
function image_close(&$image, $filename = NULL, $quality = NULL) {
  $imagelib = variable_get('image_lib', 'imagemagick');
  $function = '_image_close_' . $imagelib;

  if (function_exists($function)) {
    return $function($image, $filename, $quality);
  } else {
    return false;
  }
}

/**
 * Resize (resample) an image file.
 */
function image_scale(&$image, $width = '', $height = '') {
  $imagelib = variable_get('image_lib', 'imagemagick');
  $function = '_image_scale_' . $imagelib;

  if (function_exists($function)) {
    return $function($image, $width, $height);
  } else {
    return false;
  }
}

/**
 * Rotate an image.
 */
function image_rotate(&$image, $degrees) {
  $imagelib = variable_get('image_lib', 'imagemagick');
  $function = '_image_rotate_' . $imagelib;

  if (function_exists($function)) {
    return $function($image, $degrees);
  } else {
    return false;
  }
}

/**
 * Crop an image.
 */
function image_crop(&$image, $topx, $topy, $width, $height) {
  $imagelib = variable_get('image_lib', 'imagemagick');
  $function = '_image_crop_' . $imagelib;

  if (function_exists($function)) {
    return $function($image, $topx, $topy, $width, $height);
  } else {
    return false;
  }
}

/**
 * Crop and scale an image.
 */
function image_crop_and_scale(&$image, $width, $height, $topx, $topy) {
  $imagelib = variable_get('image_lib', 'imagemagick');
  $function = '_image_crop_and_scale_' . $imagelib;

  if (function_exists($function)) {
    return $function($image, $width, $height, $topx, $topy);
  } else if (function_exists('image_crop_' . $imagelib) && function_exists('image_scale_' . $imagelib)) {
    if (image_crop($image, $topx, $topy) && image_scale($image, $width, $height)) {
      return true;
    } else {
      return false;
    }
  } else {
    return false;
  }
}

/**
 * See if the current image library supports this image type.
 */
function image_supported_type(&$image) {
  $imagelib = variable_get('image_lib', 'imagemagick');
  $function = '_image_supported_type_' . $imagelib;

  if (function_exists($function)) {
    return $function($image);
  } else {
    return false;
  }
}

/**
 * @}
 */

/**
 * @defgroup imagemagick
 *  Image manipulation functions using ImageMagick.
 * @{
 */

function _image_open_imagemagick($filename) {
  if (file_exists($filename)) {
    $image->filename = $filename;
    return $image;
  } else {
    return false;
  }
}

function _image_close_imagemagick(&$image, $filename = NULL, $quality = NULL) {
  if ($quality) {
    $image->trasform_string .= ' -quality ' . $quality;
  }
  $image->error = _image_convert($image->filename, ($filename ? $filename : $image->filename), $image->trasform_string);
  return $image->error ? false : true;
}

function _image_scale_imagemagick(&$image, $width = '', $height = '') {
  $image->trasform_string .= ' -scale ' . $width . 'x' . $height . ' -filter QUADRATIC';
  return true;
}

function _image_rotate_imagemagick(&$image, $degrees) {
  $image->trasform_string .= ' -rotate ' . $degrees . ' -background ' . variable_get('image_background', '#000000');
  return true;
}

function _image_crop_imagemagick(&$image, $topx, $topym, $width, $height) {
  $image->trasform_string .= ' -crop ' . $width . 'x' . $height . '+' . $topx . '+' . $topy;
  return true;
}

function _image_supported_type_imagemagick(&$image) {
  return true;
}

function _image_convert($source, $dest, $filter) {
  $_imagick_convert = variable_get('image_convert_path', 'misc/image/convert.exe');
  if (!$_imagick_convert) {
    return t("Imagemagick: you have to set <code>convert</code> path.");
  }

  // 040 = space
  $filter = preg_replace("/[^A-Za-z0-9\.\-\+\040]/", '', $filter);
  $source = _image_escape_shell($source);
  $dest = _image_escape_shell($dest);
  $err = _image_exec("$_imagick_convert $filter $source $dest");  // MSChange
  return $err;
}

/**
 * @}
 */

/**
 * @defgroup libmagick
 *  Image manipulation functions using ImageMagick php extension.
 * @{
 */

function _image_open_libmagick($filename) {
  if (file_exists($filename)) {
    $image->filename = $filename;
    $image->imdata = imagick_readimage($filename);
    if (imagick_iserror($image->imdata)) {
      return false;
    }
    else {
      return $image;
    }
  } else {
    return false;
  }
}

function _image_close_libmagick(&$image, $filename = NULL, $quality = NULL) {
  if (!imagick_writeimage($image->imdata, $filename ? $filename : $image->filename)) {
    return false;
  }
  return true;
}

function _image_scale_libmagick(&$image, $width = '', $height = '') {
  if (!imagick_scale($image->imdata, $width, $height, "!")) {
    return false;
  }
  return true;
}

function _image_rotate_libmagick(&$image, $degrees) {
  $degrees = ($degrees > 360 ? fmod($degrees, 360)/90  : $degrees/90);
  if (!fmod($degrees, 90)) {
    switch($degrees) {
      case 90:
        $x = 0;
        $y = 90;
      case 180:
      case 270:
      default:
        $x = $y = 0;
    }
    if (!imagick_shear($image->imdata, $x, $y)) {
      return false;
    }
  }
  else {
    if (!imagick_rotate($image->imdata, $degrees)) {
      return false;
    }
  }
  return true;
}

function _image_crop_libmagick(&$image, $topx, $topym, $width, $height) {
  if (!imagick_crop($image->imdata, $topx, $topym, $width, $height)) {
    return false;
  }
  return true;
}

function _image_supported_type_libmagick(&$image) {
  return true;
}
/**
 * @}
 */

/**
 * @defgroup imlib2
 *  Image manipulation functions using imlib2.
 * @{
 */

function _image_open_imlib2($filename) {
  if (file_exists($filename)) {
    $image->filename = $filename;
    $image->imdata = imlib2_load_image($filename);
    if (!$image->imdata) {
      return false;
    } else {
      return $image;
    }
  } else {
    return false;
  }
}

function _image_close_imlib2(&$image, $filename, $quality = NULL) {
  if ($quality) {
    imlib2_save_image($image->imdata, ($filename ? $filename : $image->filename), $image->error, $quality);
  } else {
    imlib2_save_image($image->imdata,($filename ? $filename : $image->filename), $image->error);
  }
  return ($image->error ? false : true);
}

function _image_scale_imlib2(&$image, $width = '', $height = '') {
  $img = imlib2_create_scaled_image($image->imdata, $width, $height);
  if (!img) {
    return false;
  }
  imlib2_free_image($image->imdata);
  $image->imdata = $img;
  return true;
}

function _image_rotate_imlib2(&$image, $degrees) {
  if (!fmod($degrees, 90)) {
    imlib2_image_orientate($image->imdata, ($degrees > 360 ? fmod($degrees, 360)/90  : $degrees/90));
  } else {
    $img = imlib2_create_rotated_image($image->imdata, $degrees);
    if (!img) {
      return false;
    }
    imlib2_free_image($image->imdata);
    $image->imdata = $img;
  }
  return true;
}

function _image_crop_imlib2(&$image, $topx, $topy, $width, $height) {
  $img = imlib2_create_cropped_image($image->imdata, $topx, $topy, $width, $height);
  if (!img) {
    return false;
  }
  imlib2_free_image($image->imdata);
  $image->imdata = $img;
  return true;
}

function _image_supported_type_imlib2(&$image) {
  return true;
}

/**
 * @}
 */

/**
 * @defgroup gd1
 *  Image manipulation functions using gd library, version 1.
 * @{
 */

function _image_open_gd1($filename) {
  return _image_open_gd2($filename);
}

function _image_close_gd1(&$image, $filename = NULL, $quality = NULL) {
  return _image_close_gd2($image, $filename, $quality);
}

function _image_scale_gd1(&$image, $width = '', $height = '') {
  // X:x = Y:y
  if (!$height) {
    $height = round($image->image_details['height'] * $width/$image->image_details['width']);
  } else if (!$width){
    $width = round($image->image_details['width'] * $height/$image->image_details['height']);
  }
  $th = imageCreateTrueColor($width, $height);
  imageCopyResized($th, $image->imdata, 0, 0, 0, 0, $width, $height, $image->image_details['width'], $image->image_details['height']);
  imageDestroy($image->imdata);
  $image->imdata = $th;
  return true;
}

function _image_supported_type_gd1(&$image) {
  return _image_supported_type_gd2($image);
}

/**
 * @}
 */

/**
 * @defgroup gd2
 *  Image manipulation functions using gd library, version 2.
 * @{
 */

function _image_open_gd2($filename) {
  if (file_exists($filename)) {
    $image->filename = $filename;
    $image->image_details = image_get_info($image->filename);
    $function = 'imageCreateFrom' . $image->image_details['type'];
    if (function_exists($function)) {
      $image->imdata = $function($image->filename);
    } else {
      return false;
    }
    return $image;
  } else {
    return false;
  }
}

function _image_close_gd2(&$image, $filename = NULL, $quality = NULL) {
  $function = 'image' . $image->image_details['type'];
  if (function_exists($function))  {
    $function($image->imdata, $filename ? $filename : $image->filename);
    imageDestroy($image->imdata);
    return true;
  } else {
    return false;
  }
}

function _image_scale_gd2(&$image, $width = '', $height = '') {
  // X:x = Y:y
  if (!$height) {
    $height = round($image->image_details['height'] * $width/$image->image_details['width']);
  } else if (!$width){
    $width = round($image->image_details['width'] * $height/$image->image_details['height']);
  }
  $th = imageCreateTrueColor($width, $height);
  imageCopyResampled($th, $image->imdata, 0, 0, 0, 0, $width, $height, $image->image_details['width'], $image->image_details['height']);
  imageDestroy($image->imdata);
  $image->imdata = $th;
  return true;
}

function _image_supported_type_gd2(&$image) {
  if (function_exists('imagetypes')) {
    $types = imagetypes();
    if (($types & IMG_GIF) && $image->type == 'GIF')
      return true;
    if (($types & IMG_JPG) && $image->type == 'JPEG')
      return true;
    if (($types & IMG_PNG) && $image->type == 'PNG')
      return true;
    if (($types & IMG_WBMP) && $image->type == 'WBMP')
      return true;
  }

  return false;
}

/**
 * @}
 */

/**
 * Helper function: Escape filename for Unix or Windows shell.
 *
 * @param $filename filename to escape -- may contain spaces or other problematic characters
 *
 * @return escaped version of $filename which should pass through shell
 */

function _image_escape_shell($filename) {

  if (strstr($_SERVER['SERVER_SOFTWARE'], 'Win32') || strstr($_SERVER['SERVER_SOFTWARE'], 'IIS')) {
    // I can't make escapeshellarg work on windows, it adds a '
    // this should be safe enough
    return '"' . addslashes(_image_strip_name($filename)) . '"';
  } else {
    return escapeshellarg(_image_strip_name($filename));
  }
}

// MSChange
function _image_exec($cmd) {
  if (substr(php_uname(), 0, 7) == "Windows"){
    if ($h = popen("start \"bla\" $cmd", "r")) {
      pclose($h);
      return true;
    } else {
      return false;
    }
  } else {
    return exec($cmd);    
  }
}

/**
 * Removes any invalid characters from the filename.
 */
function _image_strip_name($filename) {
  // We don't allow filenames with strange characters.
  // We try to convert some characters, and strip others.
  $filename = strtr(urldecode($filename),
      'ÁÉÍÓÚÑÀÈÌÒÙÄËÏÖÜÂÊÎÔÛáéíóúñàèìòùäëïöüâêîôûç ',
      'aeiounaeiouaeiouaeiouaeiounaeiouaeiouaeiouc_');

  return strtolower(preg_replace("/[^A-Za-z0-9\.\-\+\/_]/", '', $filename));
}

/**
 * Returns the image types supported by the GD library.
 */
function _get_gd_image_types() {

  $supported_types = array();

  if (function_exists('imagetypes')) {
    $types = imagetypes();
    if ($types & IMG_GIF)  $supported_types[] = 'GIF';
    if ($types & IMG_JPG)  $supported_types[] = 'JPEG';
    if ($types & IMG_PNG)  $supported_types[] = 'PNG';
    if ($types & IMG_WBMP) $supported_types[] = 'WBMP';
  }
  return $supported_types;
}
?>
