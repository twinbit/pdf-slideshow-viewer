<?php
function file_check_directories($path, $mode = 0, $form_item = NULL, $temp = FALSE) {
  $dirs = array(($temp) ? file_directory_temp() : file_directory_path());
  foreach(array_filter(explode('/', trim($path, '/'))) as $dir) {
    $dir = trim($dir);
    if ($dir == '..' || $dir == '.') {
      continue;
    }
    $dirs[] = $dir;
    $path = file_create_path($directory . implode($dirs,'/'));
    $result = my_file_check_directory($path, $mode, $form_item);
    if(!$result) {
      return FALSE;
    }
  }
  return TRUE;
}


function my_file_check_directory(&$directory, $mode = 0, $form_item = NULL) {
  $directory = rtrim($directory, '/\\');

  // Check if directory exists.
  if (!is_dir($directory)) {
    if (($mode & FILE_CREATE_DIRECTORY) && @mkdir($directory)) {
     // drupal_set_message(t('The directory %directory has been created.', array('%directory' => $directory)));
      @chmod($directory, 0775); // Necessary for non-webserver users.
    }
    else {
      if ($form_item) {
        form_set_error($form_item, t('The directory %directory does not exist.', array('%directory' => $directory)));
      }
      return FALSE;
    }
  }

  // Check to see if the directory is writable.
  if (!is_writable($directory)) {
    if (($mode & FILE_MODIFY_PERMISSIONS) && @chmod($directory, 0775)) {
   //   drupal_set_message(t('The permissions of directory %directory have been changed to make it writable.', array('%directory' => $directory)));
    }
    else {
      form_set_error($form_item, t('The directory %directory is not writable', array('%directory' => $directory)));
      watchdog('file system', t('The directory %directory is not writable, because it does not have the correct permissions set.', array('%directory' => $directory)), WATCHDOG_ERROR);
      return FALSE;
    }
  }

  if ((file_directory_path() == $directory || file_directory_temp() == $directory) && !is_file("$directory/.htaccess")) {
    $htaccess_lines = "SetHandler Drupal_Security_Do_Not_Remove_See_SA_2006_006\nOptions None\nOptions +FollowSymLinks";
    if (($fp = fopen("$directory/.htaccess", 'w')) && fputs($fp, $htaccess_lines)) {
      fclose($fp);
      chmod($directory .'/.htaccess', 0664);
    }
    else {
      $message = t("Security warning: Couldn't write .htaccess file. Please create a .htaccess file in your %directory directory which contains the following lines: <code>!htaccess</code>", array('%directory' => $directory, '!htaccess' => '<br />'. nl2br(check_plain($htaccess_lines))));
      form_set_error($form_item, $message);
      watchdog('security', $message, WATCHDOG_ERROR);
    }
  }

  return TRUE;
}
?>