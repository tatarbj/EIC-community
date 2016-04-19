<?php
/**
 * @file
 * Bootstrap 8-4 template for Display Suite.
 */
?>

<<?php print $layout_wrapper; print $layout_attributes; ?> class="<?php print $classes; ?>">

<?php if (isset($title_suffix['contextual_links'])): ?>
  <?php print render($title_suffix['contextual_links']); ?>
<?php endif; ?>

<div class="content clearfix">
  <<?php print $left_wrapper; ?> class="group-left col-xs-3 <?php print $left_classes; ?>">
    <?php print $left; ?>
  </<?php print $left_wrapper; ?>>
  <<?php print $right_wrapper; ?> class="group-right col-xs-9 <?php print $right_classes; ?>">
    <?php print $right; ?>
  </<?php print $right_wrapper; ?>>
</div>

</<?php print $layout_wrapper ?>>


<!-- Needed to activate display suite support on forms -->
<?php if (!empty($drupal_render_children)): ?>
  <?php print $drupal_render_children ?>
<?php endif; ?>
