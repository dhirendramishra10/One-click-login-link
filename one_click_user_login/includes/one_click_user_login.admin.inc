<?php

/**
 * @file
 * Administration pages provided by One Click Login Link module.
 */

/**
 * Returns form to user
 * @param type $form
 * @param type $form_state
 * @param type $userid
 * @return type
 */
function one_click_user_login_user_settings($form, &$form_state, $userid) {

  // we need to override this value otherwise 
  // user will change the value of user id in url and can view anyones detail
  $userid = $GLOBALS['user']->uid;
  $form = array();

  if (!empty($userid)) {
    $user_data = one_click_user_login_get_ocul_user_details($userid);
    $form_state['user_data'] = $user_data; // store all the values in form state to pass values in form_submit

    $form['on_off_flag'] = array(
      '#title' => t('On/Off flag'),
      '#type' => 'checkbox',
      '#default_value' => !empty($user_data['on_off_flag']) ? $user_data['on_off_flag'] : 0, // needs to correct
      '#description' => t('One click Login Link. Enable it to use this feature'),
      '#required' => FALSE,
    );

    $ignore_string = array(//'-select-' => 'select',
      1 => 1,
      2 => 2,
      3 => 3,
      4 => 4,
      5 => 5,
      6 => 6,
      7 => 7,
      8 => 8,
      9 => 9,
      10 => 10,
    );

    $form['random_ignore_string'] = array(
      '#title' => t('Select any Random value from 1 to 10'),
      '#type' => 'select',
      '#options' => $ignore_string,
      '#default_value' => !empty($user_data['ignore_value']) ? $user_data['ignore_value'] : 'none', // needs to correct
      '#description' => t('Select any value from 1 to 10'),
      '#required' => TRUE,
      '#empty_option' => '- None -',
    );
    $form['random_encryption_string'] = array(
      '#title' => t('Encryption Key'),
      '#type' => 'textfield',
      '#default_value' => !empty($user_data['encrypt_key']) ? $user_data['encrypt_key'] : '', // needs to correct
      '#description' => t('Type any random value between 1 to 9999'),
      '#validate' => 'element_validate_integer_positive',
      '#required' => TRUE,
      '#maxlength' => 4,
      '#size' => 15,
    );
    $form['generate_submit'] = array(
      '#value' => t('Generate Link'),
      '#type' => 'submit',
    );
  }
  return $form;
}

/**
 * Validate handler for webform_admin_settings
 */
function one_click_user_login_user_settings_validate($form, &$form_state) {

  $random_key = $form_state['values']['random_encryption_string'];
  if (($random_key < 1) || ($random_key > 9999)) {
    form_set_error('random_encryption_string', t('kindly enter random value between 1 to 9999'));
  }
  //if (var_dump(is_float($random_key))) {
  if (strpos($random_key, ".") !== false) {
    form_set_error('random_encryption_string', t('Random Key can not be in decimal'));
  }
}

/**
 * Submit handler for webform_admin_settings
 */
function one_click_user_login_user_settings_submit($form, &$form_state) {

  global $user;
  $user_email = $user->mail;


  //$submit_values = $form_state[]
  if ($form_state['clicked_button']['#value'] == 'Generate Link') {
    $userid = $user->uid;
    $ignore_value = $form_state['values']['random_ignore_string'];
    $encrypt_key = $form_state['values']['random_encryption_string'];
    $on_off_value = $form_state['values']['on_off_flag'];
    // Logic here to generate encrypted url based on the user id.
    $encrypted_url = _generate_one_click_user_login_link($userid, $ignore_value, $encrypt_key);

    if (!empty($encrypted_url)) {

      db_merge('ocul')
          ->key(array('userid' => $userid))
          ->fields(array(
            'ignore_value' => $ignore_value,
            'encrypt_key' => $encrypt_key, // pass vraiable defined in oxigen wallet configuratation form
            'on_off_flag' => $on_off_value,
          ))
          ->execute();

      if ($on_off_value == 1) {
        // send email logic here
        _one_click_user_login_send_email($user_email, $encrypted_url);
        drupal_set_message(t('Email has been sent to your email id with one click login link. Please check'));
      }
      else {
        drupal_set_message(t('Unable to send email. Please check On/Off flag'), 'error');
      }
    }
  }
}

function _one_click_user_login_send_email($user_email, $encrypted_url) {

  if (!empty($user_email) && !empty($encrypted_url)) {

    global $conf, $user, $base_url;
    $username = $user->name;
    $site_name = $conf['site_name'];

    $site_name = $conf['site_name'];
    $params['subject'] = $site_name . ' One click Login Link';
    $params['body'] = "Hello " . $username . "," . '<br/>'
        . "Thank you for generating One click login link at $site_name. You may now log in by clicking this"
        . '<br/>'
        . "link or copying and pasting it to your browser:"
        . '<br/>'
        . $base_url . '/' . $encrypted_url
        . '<br/>'
        . '<br/>'
        . '<br/>'
        . "-- $site_name team";

    drupal_mail('one_click_user_login', 'send_login_url', $user_email, LANGUAGE_NONE, $params);
  }
}
