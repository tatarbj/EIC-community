<?php

/**
 * @file
 * C4M Admin Event Sites administration pages functionality.
 */

/**
 * Page callback for /admin/c4m/event_sites/export.
 */
function c4m_admin_event_sites_event_sites_export_page() {
  $output = '';

  // Get views view settings.
  $view = views_get_view('c4m_admin_event_sites_export');
  $view_display = $view->display['event_sites_export'];
  $view_display_options = $view_display->display_options;

  // Permanent file path (if the file can be found).
  $permanent_file_path = $view_display_options['style_options']['file_path'];
  if (file_exists($permanent_file_path)) {
    $date = _c4m_admin_event_sites_get_event_site_report_last_generated_date();

    $output .= t('<p>Event Sites export file was last generated on @date.</p>', array('@date' => $date));

    $permanent_file_path = file_create_url($permanent_file_path);
    $output .= l(t('Download report'), $permanent_file_path, array('attributes' => array('id' => 'edit-cancel')));
  }

  // New export path.
  $output .= l(t('Generate report'), $view_display_options['path'], array('attributes' => array('class' => array('button'))));

  return $output;
}

/**
 * Returns the time & date when the event_sites export file was last generated.
 *
 * @return string
 *   Date & time.
 */
function _c4m_admin_event_sites_get_event_site_report_last_generated_date() {
  $timestamp = db_query("SELECT time_stamp FROM views_data_export WHERE view_name = 'c4m_admin_event_sites_export' AND view_display_id = 'event_sites_export' ORDER BY time_stamp DESC LIMIT 0, 1")->fetchCol();
  if (!$timestamp) {
    return t('Never');
  }
  else {
    return date('Y-m-d H:i', $timestamp[0]);
  }
}
