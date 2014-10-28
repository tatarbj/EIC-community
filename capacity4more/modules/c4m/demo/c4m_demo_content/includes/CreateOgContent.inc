<?php
/**
 * @file
 * Base to create Og Content Classes
 */

/**
 * Class C4mDemoContentCreateOgContent
 */
abstract class C4mDemoContentCreateOgContent extends C4mDemoContentCreateEntity {
  public $entityType = 'node';

  /**
   * Content creations need to be run before this one.
   *
   * @var array
   */
  public $dependencies = array(
    'C4mDemoContentCreateUsers',
    'C4mDemoContentCreateGroups',
    'C4mDemoContentCreateCategories',
    'C4mDemoContentCreateTags',
  );

  /**
   * Configure the migration class during creation.
   */
  public function __construct() {
    parent::__construct();

    $this->addFieldMapping('uid', 'uid')
         ->sourceMigration('C4mDemoContentCreateUsers');

    $this->addFieldMapping(OG_AUDIENCE_FIELD, OG_AUDIENCE_FIELD);

  }

  /**
   * Prepare the row data loaded from the CSV.
   *
   * @param object $row
   *   The data from the CSV file.
   *
   * @return bool
   *   Success.
   */
  public function prepareRow($row) {
    parent::prepareRow($row);

    switch ($row->group_type) {
      case 'group':
        $unfiltered_ids = $this->handleSourceMigration('C4mDemoContentCreateGroups', $row->group_map, NULL, $this);
        break;

      case 'project':
        $unfiltered_ids = $this->handleSourceMigration('C4mDemoContentCreateProjects', $row->group_map, NULL, $this);
        break;
    }
    $row->{OG_AUDIENCE_FIELD} = $this->filterDestid2FromEntityReference(
                                     $unfiltered_ids
    );
  }

  /**
   * Map terms to the existing (install hook) terms in the database for a specified taxonomy.
   *
   * @param $sourcefield
   */
  public function mapTaxonomyVocab($sourcefield, $vocabname) {
    $vocab = taxonomy_vocabulary_machine_name_load($vocabname);

    if (!empty($vocab)) {
      $terms = explode('|', $sourcefield);
      if (!empty($terms)) {
        $tids = array();
        foreach ($terms as $term_name) {
          $term = taxonomy_get_term_by_name($term_name, $vocabname);
          if (!empty($term)) {
            reset($term);
            $tid = key($term);
            $tids[] = $tid;
          };
        }

        return $tids;
      }
    }
  }
}