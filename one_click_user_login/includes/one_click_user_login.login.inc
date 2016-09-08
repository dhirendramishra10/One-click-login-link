<?php

/**
 * @file
 * Provides interface and database handling for e-mail settings of a webform.
 *
 * @author Dhirendra Mishra <dhirenmishra10@gmail.com>
 */
function one_click_user_login_callback() {

  global $base_url, $user;
  $userid = $user->uid;

  if (isset($userid) && !empty($userid)) {
    //when already logged in with another user
    //and tries to login with one click login link in same session
    drupal_set_message(t('Unable to Login. You are already logged in with another user.'), 'error');
    drupal_goto($base_url);
    return;
  }

  $parameters = drupal_get_query_parameters();
  if (!isset($parameters['id1']) && !isset($parameters['id2']) && empty($parameters['id1']) && empty($parameters['id2'])) {
    //when parametrs are empty or not set
    //redirect to home page with error
    drupal_set_message(t('Unable to Login. Kindly contact administrator'), 'error');
    drupal_goto($base_url);
    return;
  }

  //Logic for Login checking start here
  $id1_value = $parameters['id1'];
  $id2_value = $parameters['id2'];

  $req_userid = substr($id1_value, Rand_Str_Length); //first 10 digit are random number, so skip it.
  if (!is_numeric($req_userid)) {
    // no correct user id in url
    drupal_set_message(t('Unable to Login. Kindly contact administrator.'), 'error');
    drupal_goto($base_url);
    return;
  }

  $user_data = one_click_user_login_get_ocul_user_details($req_userid);
  if (!empty($user_data)) {
    $id2_res = md5($id1_value . $user_data['encrypt_key']);
    $id2_value = substr($id2_value, $user_data['ignore_value']); // Overriding id2_value

    if ($id2_res == $id2_value) { // users data is correct and matched
      // all creadiatials are valid make autologin for currosponding user
      $user = user_load($req_userid);
      user_login_finalize();
      drupal_set_message(t('Welcome to Portal with one click login link!!!'));
      drupal_goto($base_url);
    }
    else {
      // incorrect url
      drupal_set_message(t('Unable to Login. URL is incorrect.'), 'error');
      drupal_goto($base_url);
      return;
    }
  }
  else {
    // no db record found for user id in ocul table
    drupal_set_message(t('Unable to Login. Kindly contact administrator.'), 'error');
    drupal_goto($base_url);
    return;
  }
}
