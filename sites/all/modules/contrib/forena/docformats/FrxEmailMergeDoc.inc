<?php
/**
 * @file FrxEmailMergeDoc.inc
 * email merge document.
 * @author davidmetzler
 *
 */
class FrxEmailMergeDoc extends FrxDocument {
  private $title;
  public function render($r, $format, $content = array()) {
    $body =  '<html><head><meta http-equiv="Content-Type" content="text/html; charset=UTF-8"/></head><body>' . $r->html . '</body></html>';
    $doc = new DOMDocument('1.0', 'UTF-8');
    $doc->strictErrorChecking = FALSE;
    libxml_use_internal_errors(true);
    $doc->loadHTML($body);
    libxml_clear_errors();
    $xml = simplexml_import_dom($doc);
    if (!$xml) return $content;
    $docs = $xml->xpath('.//*[@class="email-document"]');
    $prompt_subject = TRUE;
    $prompt_body = TRUE;
    if ($r->title) {
      $this->title = $r->title;
    }
    if ($docs) foreach ($docs as $doc) {

      // From
      $from = $doc->xpath('.//*[@class="email-header-from"]');
      $from = $from ? html_entity_decode(strip_tags($from[0])) : '';

      // Subject
      $subject = $doc->xpath('.//*[@class="email-header-subject"]');
      if ($subject) $prompt_subject = FALSE;
      $subject = $subject ? (string)$subject[0] : '';

      // To
      $to = $doc->xpath('.//*[@class="email-header-to"]');
      $to = $to ? html_entity_decode(strip_tags($to[0]->asXML())) : '';
      if ($to) $prompt_to = FALSE;
      $body = $doc->xpath('.//*[@class="email-body"]');
      if ($body) $prompt_body = FALSE;
      $body = $body ? $body[0]->asXML() : $body;
      // Assemble email
      $email = array(
        'to' => $to,
        'from' => $from,
        'parms' => array(
          'subject' => $subject,
          'body' => $body,
        ),
      );

      // Check for cc
      $cc = $doc->xpath('.//*[@class="email-header-cc"]');
      if ($cc) {
        $email['parms']['headers']['Cc'] = html_entity_decode(strip_tags($cc[0]->asXML()));

      }

      // Check for bcc
      $bcc = $doc->xpath('.//*[@class="email-header-bcc"]');
      if ($bcc) {
        $email['parms']['headers']['Bcc'] = html_entity_decode(strip_tags($bcc[0]->asXML()));
      }
      $emails [] = $email;
    }
    $count = count($docs);
    if ($count) {
      if ($this->print) {
        $content = drupal_get_form('forena_confirm_email', $emails, $count, $prompt_subject, $prompt_body);
      }
      else {
        $content = '';
        $this->emailDocs($emails);
      }
      drupal_set_title($this->title);
    }
    else {
      drupal_set_message(t('No mail merge information in report. Displaying report instead.'), 'error');
      $content = array(
          'content' => array('#markup' => $body),
      );
    }
    return $content;
  }

  public function emailDocs($docs, $test_mode=NULL, $max=10, $subject='', $body = '') {
    global $user;
    $test_send = $test_mode!==NULL ? $test_mode : variable_get('forena_email_override', FALSE);
    print("test send: $test_send");
    $i = 0;
    $sent = 0;
    if ($body) {
      $body =  check_markup($body, variable_get('forena_email_input_format', filter_default_format()));
    }
    print_r($docs, 'doc');
    foreach ($docs as $doc) {
      $to = $test_send ? $user->mail : $doc['to'];
      $from = $doc['from'];
      // Replace body
      if ($body) {
        $doc['parms']['body'] = $body;
      }
      // Replace subject
      if ($subject) {
        $doc['parms']['subject'] = $subject;
      }
      if ($test_send) {
        $i++;
        // Remove bcc and cc
        unset($doc['parms']['headers']);
      }
      if ($i <= $max) {
        $sent++;
        drupal_mail('forena', 'mailmerge', $to, language_default(), $doc['parms'], $from, TRUE);
      }
    }
    drupal_set_message(t('Sent %count messages ', array('%count' => $sent)));
  }

  public function output(&$output) {
    return FALSE;
  }

}