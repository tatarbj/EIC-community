<?php

/**
 * @file
 * All preprocess functionality.
 */

/**
 * Implements theme_preprocess_message().
 *
 *  Activity stream messages preprocess.
 */
function kapablo_preprocess_message(&$variables) {
  if ($variables['view_mode'] != 'activity_group' && $variables['view_mode'] != 'activity_global') {
    return;
  }

  $message = $variables['message'];
  $variables['theme_hook_suggestions'][] = 'message__activity_stream';
  $variables['theme_hook_suggestions'][] = "message__{$message->type}__activity_stream";

  $variables['content'] = $message->getText();

  // Getting the name of the icon image file from the message type.
  $icon_type = explode('__', $message->type);

  $icon_entity_type = $icon_type[1];

  $icon_name = 'missing';

  if ($icon_entity_type == 'comment') {
    $icon_name = 'comment';
  }

  if ($icon_entity_type == 'node') {
    // Icon image file name is content type of the node.
    $icon_name = $icon_type[2];

    if ($icon_name == 'discussion') {
      $message_wrapper = entity_metadata_wrapper('message', $message);
      // Icon image file name is the discussion type of the discussion node.
      $icon_name = 'discussion-' . $message_wrapper->field_node->c4m_discussion_type->value(
        );
    }
  }

  $variables['icon'] = $icon_name;
}

/**
 * Implements theme_preprocess_block().
 */
function kapablo_preprocess_block(&$variables) {
  if (isset($variables['elements']['#block']->bid) && $variables['elements']['#block']->bid == 'og_menu-og_single_menu_block') {
    // This block is the OG Menu. Add the tpl to the suggested filename list:
    $variables['theme_hook_suggestions'][] = 'block__menu__c4m_og_menu';
  }

  $block_id = $variables['block']->module . '-' . $variables['block']->delta;
  $classes = &$variables['classes_array'];

  // Add classes based on the block delta.
  switch ($block_id) {
    case 'c4m_user_profile-user_welcome':
    case 'c4m_user_profile-user_notifications':
      $classes[] = 'col-sm-12 col-md-6';
      break;

    case 'menu-c4m-og-menu':
      // Add search form.
      // Find current group ID.
      $gid = c4m_og_current_group_id();

      // Assemble search URL using current group.
      $redirect_url = url(
        'searchgroup',
        array(
          'purl' => array(
            'provider' => 'og_purl|node',
            'id' => $gid,
          ),
        )
      );

      $form = drupal_get_form(
        'c4m_search_redirect_form',
        $redirect_url,
        NULL,
        t(
          'Search in @group_type...',
          array('@group_type' => c4m_og_get_group_type(array('gid' => $gid)))
        )
      );

      $variables['search_form'] = $form;

      break;
  }
}

/**
 * Implements theme_preprocess_page().
 *
 * @see page.tpl.php
 * @see page--og--project.tpl.php
 */
function kapablo_preprocess_page(&$variables) {
  // Determine if the current context is a project.
  if (($group_type = c4m_og_get_group_type(og_context())) && $group_type == 'project') {
    // Instruct to use a page template designated for project OG type.
    $variables['theme_hook_suggestions'][] = 'page__og__project';
  }

  // Add information about the number of sidebars.
  if (!empty($variables['page']['sidebar_first']) && !empty($variables['page']['sidebar_second'])) {
    $variables['content_column_class_kapablo'] = ' class="col-sm-4"';
  }
  elseif (!empty($variables['page']['sidebar_first']) || !empty($variables['page']['sidebar_second'])) {
    $variables['content_column_class_kapablo'] = ' class="col-sm-8"';
  }
  else {
    $variables['content_column_class_kapablo'] = ' class="col-sm-12"';
  }

  $variables['front_page'] = url('<front>', array('purl' => array('disabled' => TRUE)));

  // Add search form.
  $variables['search_form'] = drupal_get_form('c4m_search_form');

  $variables['image_banner'] = kapablo_group_image_banner();
  $variables['image_banner_background'] = kapablo_group_image_banner_background();

  // Take care of the breadcrumb for OG content.
  _set_og_breadcrumbs();

  // Off-canvas trigger labels.
  $offcanvas_trigger_label_left = array('icon' => 'fa fa-info', 'label' => t('More info'));
  $offcanvas_trigger_label_right = array('icon' => 'fa fa-info', 'label' => t('More info'));

  drupal_alter('c4m_offcanvas_trigger_label_left', $offcanvas_trigger_label_left);
  drupal_alter('c4m_offcanvas_trigger_label_right', $offcanvas_trigger_label_right);

  $variables['offcanvas_trigger_label_left'] = $offcanvas_trigger_label_left;
  $variables['offcanvas_trigger_label_right'] = $offcanvas_trigger_label_right;
}

/**
 * Helper function to set the breadcrumb for OG content.
 */
function _set_og_breadcrumbs() {
  // We use the raw path (purl pages do not have system paths).
  $path_elements = explode('/', request_path());

  // If the current path contains 2 or less parts and
  // the first part is a Persistent URL.
  if (count($path_elements) <= 2 && $purl = purl_load(array(
    'provider' => 'og_purl|node',
    'value' => $path_elements[0],
  ))) {
    $breadcrumb = array();

    // We need to load the group to get the
    // group title to use in the breadcrumb.
    $group = node_load($purl['id']);

    // Just have the group title displayed on group pages.
    if (count($path_elements) == 1) {
      $breadcrumb[] = $group->title;
    }
    // This deals with the direct menu items in the group menu.
    else {
      // We use the menu trail to get the titles for the breadcrumb.
      $trail = menu_get_active_trail();
      if (isset($trail[1]['menu_name']) && $trail[1]['menu_name'] == 'c4m-og-menu') {
        // Breadcrumb element linking to the group page.
        $breadcrumb[] = l($group->title, $path_elements[0]);
        $breadcrumb[] = $trail[1]['link_title'];
      }
    }

    drupal_set_breadcrumb($breadcrumb);
  }
}

/**
 * Implements template_preprocess_field().
 */
function kapablo_preprocess_field(&$variables, $hook) {
  $element = $variables['element'];

  switch ($element['#field_name']) {
    case 'c4m_ds_group_access_info':
      $group = $element['#object'];

      // Retrieve group access info.
      $group_access = c4m_og_get_access_type($group);

      if (!empty($group_access['type'])) {
        $variables['classes_array'][] = 'group-' .
          drupal_html_class($group_access['type']);
      }
      break;

    // Group status field,
    // Do NOT display when the status is "Published".
    case 'c4m_og_status':
      if (reset($variables['items'])['#markup'] == 'Published') {
        unset($variables['items'][0]);
        // Remove the class.
        unset($variables['ds-config']['fis-cl']);
      }
      break;

    case 'c4m_ds_group_members_count':
      foreach ($element['#items'] as $key => $item) {
        $variables['items'][$key]['#markup'] = '<span class="group-statistic"><i class="fa fa-user"></i> ' . $variables['items'][$key]['#markup'] . '</span>';
      }
      break;

    case 'c4m_ds_group_posts_count':
      foreach ($element['#items'] as $key => $item) {
        $variables['items'][$key]['#markup'] = '<span class="group-statistic"><i class="fa fa-comments-o"></i> ' . $variables['items'][$key]['#markup'] . '</span>';
      }
      break;

    case 'c4m_discussion_type':
      switch ($element['#view_mode']) {
        case 'full':
          foreach ($element['#items'] as $key => $item) {
            $icon_class = c4m_content_discussion_icon_class(
              $variables['items'][$key]['#markup']
            );

            $variables['items'][$key]['#markup'] = '<i class="fa ' . $icon_class . '"></i> ' . $variables['items'][$key]['#markup'];
          }
          break;
      }
      break;

    case 'og_vocabulary':
      foreach ($variables['items'] as $key => $item) {
        if (is_array($variables['items'][$key])) {
          foreach ($variables['items'][$key] as $subkey => $list) {
            $list['#title'] = str_replace(
              'Categories',
              'Group Categories',
              $list['#title']
            );

            $variables['items'][$key][$subkey] = $list;
          }
        }
      }
      break;

    case 'title':
      if ($element['#object']->type == 'photo') {
        $node = $element['#object'];
        $galleries = node_gallery_api_get_relationships(NULL, $node->nid);

        if (!empty($galleries)) {
          $gallery = array_pop($galleries);
          $album = node_load($gallery->ngid);
        }

        foreach ($element['#items'] as $key => $item) {
          $variables['items'][$key]['#markup'] = '<h1 class="text--inline">' . $album->title . '</h1><h2 class="text--inline">&nbsp;&gt; ' . $node->title . '</h2>';
        }
      }

      break;
  }
}

/**
 * Implements hook_preprocess_facetapi_link_inactive().
 */
function kapablo_preprocess_facetapi_link_inactive(&$variables) {
  $variables['text'] = _kapablo_help_facetapi_link_add_icon($variables['text']);
  $variables['options']['html'] = TRUE;
}

/**
 * Implements hook_preprocess_facetapi_link_active().
 */
function kapablo_preprocess_facetapi_link_active(&$variables) {
  $variables['text'] = _kapablo_help_facetapi_link_add_icon($variables['text']);
  $variables['options']['html'] = TRUE;
}

/**
 * Create replacement array for the faceted items label strings for discussions.
 *
 * @param string $text_orig
 *   The original text.
 *
 * @return array
 *   The replace info.
 */
function _kapablo_help_facetapi_link_add_icon($text_orig) {
  if ($class = c4m_content_discussion_icon_class($text_orig)) {
    return '<i class="fa ' . $class . '"></i>' . $text_orig;
  }

  return $text_orig;
}

/**
 * Implements hook_preprocess_views_view().
 *
 * Set  parameters for the custom template of the
 * c4m_overview_inthe_picture_blocks view.
 *
 * @see views-view--c4m-overview-inthe-picture-blocks--block.tpl.php
 */
function kapablo_preprocess_views_view(&$vars) {
  switch ($vars['name']) {
    case 'c4m_overview_inthe_picture_blocks':
      $items = $vars['view']->result;
      $carousels = array();
      foreach ($items as $node) {
        $wrapper = entity_metadata_wrapper('node', $node->nid);
        $banner = $wrapper->c4m_media->value();
        $link = $wrapper->c4m_link->value();
        $body = $wrapper->c4m_body->value();
        $carousels[] = array(
          'id' => $wrapper->getIdentifier(),
          'title' => $wrapper->c4m_heading->value(),
          'text' => $body['value'],
          'image' => file_create_url($banner['uri']),
          'alias' => $link['url'],
        );
      }

      // Pass info via Drupal.settings.
      $settings['c4m'] = array(
        'carousels' => $carousels,
      );
      drupal_add_js($settings, 'setting');

      break;
  }
}

/**
 * Overrides template_preprocess_views_view_fields().
 */
function kapablo_preprocess_views_view_fields(&$vars) {
  $view = $vars['view'];

  switch ($view->name) {
    case 'c4m_er_groups':
      $node = node_load($vars['row']->nid);
      $access = c4m_og_get_access_type($node);

      $object = new stdClass();
      $object->content = '<span class="er--field-access privacy-level--' . $access['type'] . '">[' . $access['type'] . ']</span>';
      $object->wrapper_prefix = '';
      $object->wrapper_suffix = '';
      $object->label_html = '';

      $vars['fields']['access'] = $object;

      break;
  }
}

/**
 * Overrides theme_breadcrumb() and bootstrap_breadcrumb().
 *
 * Print breadcrumbs as an ordered list.
 * We need to set the icon class on every item except the first one.
 * That is why we cannot use theme_item_list.
 *
 * @see bootstrap_breadcrumb()
 * @see theme_breadcrumb()
 */
function kapablo_breadcrumb($variables) {
  $output = '';
  $breadcrumb = $variables['breadcrumb'];

  // Determine if we are to display the breadcrumb.
  $bootstrap_breadcrumb = theme_get_setting('bootstrap_breadcrumb');
  if (($bootstrap_breadcrumb == 1 || ($bootstrap_breadcrumb == 2 && arg(
          0
        ) == 'admin')) && !empty($breadcrumb)
  ) {

    $output = "<ol" . drupal_attributes(
        array('class' => array('breadcrumb'))
      ) . '>';

    array_unshift(
      $breadcrumb,
      l(t('EUROPA'), 'http://ec.europa.eu'),
      l(t('capacity4dev'), '<front>', array('purl' => array('disabled' => TRUE)))
    );

    $num_items = count($breadcrumb);

    foreach ($breadcrumb as $i => $item) {
      $attributes = array();
      $i++;
      $data = '';

      if (is_array($item)) {
        foreach ($item as $key => $value) {
          if ($key == 'data') {
            $data = $value;
          }
          else {
            $attributes[$key] = $value;
          }
        }
      }
      else {
        $data = $item;
      }

      if ($i == 1) {
        $attributes['class'][] = 'first';
      }
      if ($i == $num_items) {
        $attributes['class'][] = 'last';
      }
      else {
        $data .= '<i class="fa fa-chevron-right"></i>';
      }

      $output .= '<li' . drupal_attributes(
          $attributes
        ) . '>' . $data . "</li>\n";
    }

    $output .= '</ol>';
  }
  return $output;
}

/**
 * Implements theme_item_list().
 *
 * Get back first and last classes (Bootstrap theme removes them).
 */
function kapablo_item_list($variables) {
  $items = $variables['items'];
  $title = $variables['title'];
  $type = $variables['type'];
  $attributes = $variables['attributes'];

  // Only output the list container and title, if there are any list items.
  // Check to see whether the block title exists before adding a header.
  // Empty headers are not semantic and present accessibility challenges.
  $output = isset($variables['hide_wrapper']) ? '' : '<div class="item-list">';
  if (isset($title) && $title !== '') {
    $output .= '<h3>' . $title . '</h3>';
  }

  if (!empty($items)) {
    $output .= "<$type" . drupal_attributes($attributes) . '>';
    $num_items = count($items);
    $i = 0;
    foreach ($items as $item) {
      $attributes = array();
      $children = array();
      $data = '';
      $i++;
      if (is_array($item)) {
        foreach ($item as $key => $value) {
          if ($key == 'data') {
            $data = $value;
          }
          elseif ($key == 'children') {
            $children = $value;
          }
          else {
            $attributes[$key] = $value;
          }
        }
      }
      else {
        $data = $item;
      }
      if (count($children) > 0) {
        // Render nested list.
        $data .= theme_item_list(
          array(
            'items' => $children,
            'title' => NULL,
            'type' => $type,
            'attributes' => $attributes,
          )
        );
      }
      if ($i == 1) {
        $attributes['class'][] = 'first';
      }
      if ($i == $num_items) {
        $attributes['class'][] = 'last';
      }
      $output .= '<li' . drupal_attributes(
          $attributes
        ) . '>' . $data . "</li>\n";
    }
    $output .= "</$type>";
  }
  $output .= isset($variables['hide_wrapper']) ? '' : '</div>';
  return $output;
}

/**
 * Overrides theme_menu_tree().
 */
function kapablo_menu_tree__small(&$variables) {
  return '<ul class="text-small">' . $variables['tree'] . '</ul>';
}

/**
 * Pre-processes variables for the "image" theme hook.
 *
 * See theme function for list of available variables.
 *
 * @see theme_image()
 *
 * @ingroup theme_preprocess
 */
function kapablo_preprocess_image(&$variables) {
  // Remove .img-responsive class for icon_small styled images.
  if (!empty($variables['style_name']) && $variables['style_name'] == 'icon_small') {
    if (($key = array_search('img-responsive', $variables['attributes']['class'])) !== FALSE) {
      unset($variables['attributes']['class'][$key]);
    }
  }
}

/**
 * Overrides template_process_html().
 */
function kapablo_preprocess_html(&$variables) {
  // Render off_canvas into top level variables.
  $variables['off_canvas'] = drupal_render($variables['page']['off_canvas']);
}

/**
 * Pre-processes variables for the "bootstrap_panel" theme hook.
 *
 * In case of a bootstrap panel the title is not being rendered under the
 * kapablo_form_element_label() and at this point the title is already
 * rendered, hence we're adding the tooltip element explicitly.
 *
 * @see bootstrap_preprocess_bootstrap_panel()
 */
function kapablo_preprocess_bootstrap_panel(&$variables) {
  $element = &$variables['element'];

  if (isset($element['#description']) && !isset($element['#what_is_this'])) {
    // Adding the tooltip element.
    $element['#title'] .= kapablo_render_what_is_this($element['#description']);
    // Removing the description anyway to prevent duplications.
    $element['#description'] = NULL;
  }

  bootstrap_preprocess_bootstrap_panel($variables);
}

/**
 * Implements hook_preprocess_views_exposed_form().
 *
 * Add tooltip for the label (Search) of the full text search input.
 */
function kapablo_preprocess_views_exposed_form(&$vars) {
  if (!isset($vars['form']['#info']['filter-search_api_views_fulltext'])) {
    return;
  }

  $vars['widgets']['search']->label .= theme('kapablo_search_advanced_help');
}

/**
 * Create the image banner for groups.
 *
 * @param bool $background_image
 *   Determine if the current render is for the background image or not.
 *
 * @return string
 *   The image markup.
 */
function kapablo_group_image_banner($background_image = FALSE) {
  // No OG context.
  if (!$group = c4m_og_current_group()) {
    return FALSE;
  }

  // First establish the current group.
  $wrapper = entity_metadata_wrapper('node', $group);

  $menu_item = menu_get_item();
  $markup = '';
  if (drupal_is_front_page() || ($menu_item['path'] == 'manage' && og_user_access('node', $group->nid, 'administer group'))) {
    // Add group image.
    $file = $wrapper->c4m_banner->value();
    if (!empty($file)) {
      $file['style_name'] = 'banner';
      $file['path'] = $file['uri'];
      $file['attributes'] = array('class' => 'group-banner');

      // If current page is the 'Group Management' page, and the current
      // rendering is not for the background image of the banner then add an
      // edit link to the banner.
      if (!$background_image && $menu_item['path'] == 'manage') {
        $edit_link['element'] = array(
          '#tag' => 'span',
          '#attributes' => array(
            'class' => array('edit-link'),
          ),
          '#value' => t('Edit'),
        );
        $options = array(
          'html' => TRUE,
          'attributes' => array(
            'class' => array(
              'edit-banner',
            ),
          ),
        );
        $markup .= l(theme_html_tag($edit_link) . theme_image_style($file), url('node/' . $group->nid . '/edit', array('absolute' => TRUE, 'fragment' => 'edit-image-banner')), $options);
      }
      else {
        $markup .= theme('image_style', $file);
      }
    }
  }

  return $markup;
}

/**
 * Returns the markup for the background image of the banner.
 *
 * @see kapablo_group_image_banner()
 */
function kapablo_group_image_banner_background() {
  return kapablo_group_image_banner(TRUE);
}

/**
 * Implements template_preprocess_user_profile().
 */
function kapablo_preprocess_user_profile(&$variables) {
  $block = block_load('c4m_user_profile', 'user_my_profile_groups');
  $renderable_blocks = _block_render_blocks(array($block));
  $renderable_array = _block_get_renderable_array($renderable_blocks);
  $variables['right'] = render($renderable_array);
}