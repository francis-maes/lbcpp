<!DOCTYPE html PUBLIC "-//W3C//DTD XHTML 1.0 Strict//EN"
    "http://www.w3.org/TR/xhtml1/DTD/xhtml1-strict.dtd">
<html>
<head>
	<meta charset="utf-8">
	<title>x3CysBridges Predictor</title>
	<style>
  HTML, BODY
  {
  	margin: 0;
  	padding: 0;
  	height: 100%;
  	font-family: Courier;
  	font-size: 14px;
  	text-align: justify;
  	background-color: #F8E6E0;
  }

  BODY DIV#page
  {
  	width: 720px;
  	min-height: 100%;
  	margin: 0 auto;
  	margin-top: 0px;
  	padding-right: 30px;
  	padding-left: 30px;
  	padding-bottom: 0px;
  	background-color: white;
  	border-width: 3px;
  	border-color: #DF7401;
  	border-right-style: solid;
  	border-left-style: solid;
  }
  H1
  {
    padding-top: 30px;
    margin-top: 0px;
    color: #DF7401;
    text-align: center;
  }
  H2
  {
    color: #DF7401;
  }
  A, A:hover, A:visited, A:active
  {
    color: #DF7401;
  }
  LABEL
  {
    display: block;
    margin-top: 5px;
    margin-bottom: 5px;
    color: #DF7401;
    font-weight: bold;
    font-style: italic;
  }
  INPUT
  {
    width: 195px;
    margin-left: 110px;
    border-style: solid;
    border-width: 1px;
    border-color: #DF7401;
  }
  INPUT[type="submit"]
  {
    background-color: white;
    font-style: italic;
    font-weight: bold;
    background-color: #F8E6E0;
  }
  TEXTAREA
  {
    width: 500px;
    height: 70px;
    margin-left: 110px;
    border-style: solid;
    border-width: 1px;
    border-color: #DF7401;
  }
	</style>
	</head>
	<body>
	<div id="page">

	 <h1>x3CysBridges</h1>

<?php

  $isOk['email'] = false;
  $isOk['sequence'] = false;

  if (isset($_POST['email']))
    $isOk['email'] = filter_var($_POST['email'], FILTER_VALIDATE_EMAIL); 
  
  if (isset($_POST['sequence']))
  {
    $seq = preg_replace('/\s+/', '', $_POST['sequence']); 
    $seq = strtoupper($seq);
    if (ctype_upper($seq))
      $isOk['sequence'] = preg_replace('/[BJOUXZ]/', 'X', $seq);
  }

  if (!$isOk['email'] || !$isOk['sequence'])
  {
?>
  <p><em>x3CysBridges</em> is a tool designed for biologists that attempt to determine the three-dimensional structure of protein molecules. Based on the fact that disulfide bridges add strong constraints to the native structure, the main function of <em>x3CysBridges</em> is to predict the disulfide bonding probability of each cysteine-pair of a protein and to propose a disulfide connectivity pattern that maximize these probabilities.

  <h2>Online service</h2>
  <p>In order to make a prediction, you can fill out the form with your amino acid sequence and the outcome of your request will be sent you by mail. The service is free of charge.</p>

  <form id="query" method="POST" action="index.php">
    <label>Amino acid sequence<?php if (isset($_POST['sequence']) && !$isOk['sequence']) echo " (Error - Please verify your sequence)"; ?></label>
    <textarea name="sequence"><?php if (isset($_POST['sequence'])) echo $_POST['sequence']; ?></textarea>
    <label>E-mail address<?php if (isset($_POST['email']) && !$isOk['email']) echo " (Error - Please verify your e-mail address)"; ?></label>
    <input type="text" name="email" value="<?php if (isset($_POST['email'])) echo $_POST['email']; ?>" /><input type="submit" value="Make prediction" />
  </form>

  <h2>Method</h2>
  <p>This section give an overall view of the approach used by <em>x3CysBridges</em>. For a more detailed description, we refer the reader to our article "<em>On the relevance of sophisticated features for disulfide connectivity pattern prediction</em>" (In submitted).</p>

  <p>Given an input primary structure, the disulfide pattern prediction problem consists in predicting the set of disulfide bridges appearing in the tertiary structure. This problem can be formalized as a link prediction problem in a graph whose nodes are cysteine residues, under the constraint that a given cysteine is at most linked to another one. To solve this problem, we use a pipeline composed of three steps. First, we enrich the primary structure using evolutionary information. Second, we apply a binary classifier to each pair of cysteines to estimate disulfide bonding probabilities. Finally, we use a maximum weight graph matching algorithm to extract a disulfide pattern that maximizes these probabilities.</p>
  <p>The evolutionary information, in the form of a position-specific scoring matrix, is obtained by running three iterations of the PSI-BLAST program on the non-redundant NCBI database. The binary classifier is an ensemble of 1,000 fully developped extremely randomized tree. The disulfide bonding probability of a pair of cysteines is then obtained by averaging the prediction of each of the 1,000 trees.</p>

<?php
  }
  else
  {
    $job = time();
    while (file_exists('../email/'.$job))
    {
      sleep(1);
      $job = time();
    }

    $fid = fopen('../email/'.$job, 'w+');
    fwrite($fid, $isOk['email']);
    fclose($fid);

    $fid = fopen('../fasta/'.$job, 'w+');
    fwrite($fid, $isOk['sequence']);
    fclose($fid);

    @mail("J.Becker@ulg.ac.be", "[x3CysBridges] New Job $job", 'Sent by '.$isOk['email'].'

'.$isOk['sequence'], "From: x3CysBridges Predictor <>");
?>
  <h2>Job</h2>
  <p>Your job has been submitted ! </p>
  <label>Your job identifier</label>
  <p><b><?php echo $job; ?></b></p>
  <p>The outcome of your request will be sent you by mail.</p>
  <p>The program may takes some time to run 3 iterations of PSI-BLAST and make predictions. Please be patient if you have input a long sequence.</p>
  <div align="right"><a href="index.php">Back to x3CysBridges</a></div>

  <h2>Query</h2>
  <label>E-mail address</label>
  <p><?php echo $isOk['email']; ?></p>
  <label>Amino acid sequence</label>
  <p><?php echo $isOk['sequence']; ?></p>

<?php
  }
?>

  <h2>How to cite us</h2>
  <p>Julien Becker, Francis Maes, Louis Wehenkel. "On the relevance of sophisticated features for disulfide connectivity pattern prediction"</p>
  <br />
	</div>
	</body>
</html>
