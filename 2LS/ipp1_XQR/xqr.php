#!/usr/bin/php
<?php
#XQR:xbarna01

	mb_internal_encoding("UTF-8");

	define("EOK", 0);
	define("EWARGS", 1);
	define("EINPUTO", 2);
	define("EOUTPUT", 3);
	define("EINPUTF", 4);
	define("EWQUERY", 80);



	// Parses script arguments
	$args = parseCommLineArgs($argc, $argv);
	if($args["help"] != null){
		printHelp();
		exit(EOK);
	}


	// Reads XML file from source into $xml variable
	if($args["input"] == null)
		$args["input"] = 'php://stdin';
	if($args["input"] != null && !is_readable($args["input"])){
		fprintf(STDERR, "ERROR! Input file could not be open!\n");
		exit(EINPUTO);
	}
	$xml = simplexml_load_file($args["input"], 'SimpleXMLElement', LIBXML_NOERROR);
	if($xml === false){
		fprintf(STDERR, "ERROR! Input is in an an illegal format!\n");
		exit(EINPUTF);
	}
	
	// Opens file for output
	$out = STDOUT;
	if($args["output"] != null && $args["output"] != "STDOUT"){
		if(file_exists($args["output"]) && !is_writable($args["output"])){
			fprintf(STDERR, "ERROR! Requested file for output could not be open!\n");
			exit(EOUTPUT);
		}
		$out = fopen($args["output"],"w");
		if($out == false){
			fprintf(STDERR, "ERROR! Requested file for output could not be open!\n");
			exit(EOUTPUT);
		}
	}

	parseQuery($args);

	$outelements = array();
	if($args["query"]["from"] !== false){
		// Selects elements matching query
		$filter = selectFrom($args, $xml);
		foreach ($filter as $element)
			if(eval('return('.$args["query"]["con"].');'))
				array_push($outelements, $element);

		// Orders elements
		if($args["query"]["ord"] != null){
			usort($outelements, 'cmpElemValues');
			for($i = 0; $i < count($outelements); $i++)
				$outelements[$i]->addAttribute("order",$i+1);
		}
	}
	

	// Prints elements matching query to the output file
	if(!$args["n"])
		fprintf($out, '<?xml version="1.0" encoding="utf-8"?>'."\n");
	if($args["root"] !== null)
		fprintf($out, "<".$args["root"].">\n");
	if($args["query"]["limit"] != null)
		$oelems = 0;
	foreach($outelements as $element){
		if($args["query"]["limit"] !== null){
			if($oelems >= $args["query"]["limit"])
				break;
			$oelems++;
		}
		fprintf($out, $element->asXML()."\n");
	}
	if($args["root"] != null)
		fprintf($out, "</".$args["root"].">\n");

	if($out != STDOUT)
		fclose($out);


///////////////////////////////////////////////////////////////////////////////
	// Functions
	///////////////////////////////////////////////////////////////////////////


	/*!
	 * Parses script arguments into an array, which is then returned from the function.
	 * @param argc number of arguments passed.
	 * @param argv an array of arguments.
	 * @return An array containing arguments the script was launched with
	 */
	function parseCommLineArgs($argc, $argv){
		$args = array(
			"help" => false,
			"input" => null,
			"output" => null,
			"query" => null,
			"n" => false,
			"root" => null,
		);

		// Query file variable
		$qf = null;

		// No arguments passed
		if($argc == 1){
			fprintf(STDERR, "ERROR! The script was launched with an illegal combination of arguments\n");
			printHelp();
			exit(EWARGS);
		}

		// Cycle, in which arguments are parsed
		for($i = 1; $i < $argc; $i++){
			if($args["help"] !== false){
				fprintf(STDERR, "ERROR! The script was launched with an illegal combination of arguments\n");
				exit(EWARGS);
			} 
			elseif($argv[$i] == "--help"){
				if($i > 1){
					fprintf(STDERR, "ERROR! The script was launched with an illegal combination of arguments\n");
					exit(EWARGS);
				}

				$args["help"] = true;
			} 
			elseif($argv[$i] == "-n"){
				if($args["n"]){
					fprintf(STDERR, "ERROR! The script was launched with an illegal combination of arguments\n");
					exit(EWARGS);
				}

				$args["n"] = true;
			} 
			elseif(substr($argv[$i], 0, 8) == "--input="){
				if($args["input"] != null){
					fprintf(STDERR, "ERROR! The script was launched with an illegal combination of arguments\n");
					exit(EWARGS);
				}
				else if(strlen($argv[$i]) == 8)
					$args["input"] = 'php://stdin';
				else
					$args["input"] = substr($argv[$i],8);
			} 
			elseif(substr($argv[$i], 0, 9) == "--output="){
				if($args["output"] != null){
					fprintf(STDERR, "ERROR! The script was launched with an illegal combination of arguments\n");
					exit(EWARGS);
				}
				else if(strlen($argv[$i]) == 9)
					$args["output"] = 'STDOUT';
				else
					$args["output"] = substr($argv[$i],9);
			} 
			elseif(substr($argv[$i], 0, 8) == "--query="){
				if($args["query"] != null){
					fprintf(STDERR, "ERROR! The script was launched with an illegal combination of arguments\n");
					exit(EWARGS);
				}
				else if(strlen($argv[$i]) == 8)
					$args["query"] = '';
				else
					$args["query"] = substr($argv[$i],8);
			} 
			elseif(substr($argv[$i], 0, 5) == "--qf="){
				if($qf != null){
					fprintf(STDERR, "ERROR! The script was launched with an illegal combination of arguments\n");
					exit(EWARGS);
				}
				else if(strlen($argv[$i]) == 5)
					$args["query"] = '';
				else
					$qf = substr($argv[$i],5);
			} 
			elseif(substr($argv[$i], 0, 7) == "--root="){
				if($args["root"] != null || strlen($argv[$i]) == 7){
					fprintf(STDERR, "ERROR! The script was launched with an illegal combination of arguments\n");
					exit(EWARGS);
				}

				$args["root"] = substr($argv[$i],7);
			} 
			else{
				fprintf(STDERR, "ERROR! The script was launched with an illegal combination of arguments\n");
				exit(EWARGS);
			}
		}

		if($args["help"] !== false)
			return $args;



		// Checks for a collision of --qf and --query arguments
		if($args["query"] != null and $qf != null){
			fprintf(STDERR, "ERROR! The script was launched with an illegal combination of arguments\n");
			exit(EWARGS);
		}
		else if($args["query"] == null and $qf == null){
			fprintf(STDERR, "ERROR! Wrong query\n");
			exit(EWQUERY);
		}

		// Loads query from file
		if($qf != null){
			if($qf == ""){
				fprintf(STDERR, "ERROR! Wrong query\n");
				exit(EWQUERY);
			}
			elseif(!file_exists($qf) || !is_readable($qf)){
				fprintf(STDERR, "ERROR! File with query could not be open!\n");
				exit(EWQUERY);
			}
			$args["query"] = file_get_contents($qf);
		}
		
		if($args["query"] == null || $args["query"] == ""){
			fprintf(STDERR, "ERROR! Wrong query\n");
			exit(EWQUERY);
		}

		return $args;
	}


	/*!
	 * Prints help message.
	 */
	function printHelp(){
		echo "Executes SQL-like SELECT query on XML file.\n";
		echo "Arguments:\n";
		echo " --help          - prints this message\n";
		echo " --input=<file>  - specifies input XML file\n";
		echo " --output=<file> - specifies output file\n";
		echo " --query='query' - query to be perfomed, cannot be used with --qf\n";
		echo " --qf=<file>     - specifies file containing query, cannot be used with --query\n";
		echo "  -n             - XML header is not generated in the output\n";
		echo ' --root="string" - specifies name of the root element in the output'."\n";
	}


	/*!
	 * Parses query into an array, which is then saved into arguments array.
	 * @param args An array containing information about arguments, gained from parseCommLineArgs function.
	 */
	function parseQuery(&$args){
		// Query Array - contains information required to run query
		$qarr = array(
			"select" => null,
			"limit" => null,
			"from" => null,
			"con" => null,
			"ord" => null,
		);


		// Splits query request into an array
		$tmparr = preg_split("/\s+/", $args["query"]);

		$qminlen = 4; // minimal length of valid query
		$i = 0; // index of actually parsed array element

		if($tmparr[$i] == ""){
			$i++;
			$qminlen++;
		}

		if(count($tmparr) < $qminlen){
			fprintf(STDERR, "ERROR! Wrong query\n");
			exit(EWQUERY);
		}

		if($tmparr[$i] != "SELECT"){
			fprintf(STDERR, "ERROR! Wrong query\n");
			exit(EWQUERY);
		}
		$qarr["select"] = $tmparr[$i+1];
		$i += 2;

		if($tmparr[$i] == "LIMIT"){
			if(is_numeric($tmparr[$i+1]))
				$qarr["limit"] = intval($tmparr[$i+1]);
			else{
				fprintf(STDERR, "ERROR! Wrong query\n");
				exit(EWQUERY);
			}
			$i += 2;
			if(count($tmparr) < $qminlen + 2){
				fprintf(STDERR, "ERROR! Wrong query\n");
				exit(EWQUERY);
			}
		}

		if($tmparr[$i] != "FROM"){
			fprintf(STDERR, "ERROR! Wrong query\n");
			exit(EWQUERY);
		}
		else{
			/* Occurs when FROM element is missing, causing empty output XML file
			 * (or only a header and/or an empty root element depending on arguments)
			 */
			if($tmparr[$i+1] == "WHERE"){
				$qarr["from"] = false;
				$i--;
				$args["query"] = $qarr;
				return;
			}
			else $qarr["from"] = $tmparr[$i+1];
		}
		$qarr["con"] = array_slice($tmparr,$i+2);
		unset($i);
		unset($tmparr);

		parseCondOrderClause($qarr);

		$args["query"] = $qarr;
	}



	/*!
	 * Complex function, which parses condition and order-by clauses.
	 * First, two clauses are separated and order-by clause is processed.
	 * Then condition is translated into a valid PHP expression, which will be
	 * later used for condition evaluating. During the translation process is a
	 * condition skeleton array being constructed. After the condition is
	 * translated, function checkConditionSyntax to check whether the syntax of
	 * the condition is valid.
	 * @param qarr args["query"], containing the condition and order-by clauses in an array
	 */
	function parseCondOrderClause(&$qarr){
		// Removes possible empty element if neither a condition, nor order-by clause is passed
		if(count($qarr["con"]) == 1 && $qarr["con"][0] == "")
			$qarr["con"] = array_splice($qarr["con"], 1);

		// If the condition is missing, every condition check is evaluated as true
		if(count($qarr["con"]) == 0)
			$qarr["con"] = 'true';
		else{
			// Array is joined into a string and then split into condition and order-by clause
			$qarr["con"] = implode(" ",$qarr["con"]);
			$qarr["con"] = preg_split("/(^|\s+)ORDER\s+BY\s+/", $qarr["con"]);

			// Processing of the order-by clause
			if(count($qarr["con"]) == 2){
				$qarr["ord"] = $qarr["con"][1];
				$qarr["ord"] = preg_split("/\s+/", $qarr["ord"]);
				if(count($qarr["ord"]) == 3 && $qarr["ord"][2] == "")
					$qarr["ord"] = array_slice($qarr["ord"], 0, 2);
				if(count($qarr["ord"]) != 2 || ($qarr["ord"][1] != "ASC" && $qarr["ord"][1] != "DESC")){
					fprintf(STDERR, "ERROR! Wrong query\n");
					exit(EWQUERY);
				}
				if($qarr["ord"][0][0] != "_" && !ctype_alpha($qarr["ord"][1][0])){
					fprintf(STDERR, "ERROR! Wrong query\n");
					exit(EWQUERY);
				}
			}
			$qarr["con"] = $qarr["con"][0];

			// Condition processing
			if($qarr["con"] != ""){
				// Operators are replaced with classic logical operators, spaces are added to operators
				// for easier separation of condition parts, into which is the condition then split
				$qarr["con"] = strtr($qarr["con"], array("NOT" => " ! ", "AND" => " && ", "OR" => " || ",
				 "(" => " ( ", ")" => " ) ", "<" => " < ", ">" => " > ", "=" => " == "));
				$qarr["con"] = preg_split('/("[^"]*")|(\s+)/', $qarr["con"], 0,
				               PREG_SPLIT_NO_EMPTY | PREG_SPLIT_DELIM_CAPTURE);
				$qarr["con"] = array_filter($qarr["con"], 'isNotSpace');
				$qarr["con"] = array_values($qarr["con"]);

				// Checks for the presence of 'WHERE' keyword and whether the rest of the clause is not empty
				if($qarr["con"][0] == "WHERE"){
					$qarr["con"] = array_slice($qarr["con"], 1);
					if(count($qarr["con"]) == 0 || (count($qarr["con"]) == 1 && $qarr["con"][0] == "")){
						fprintf(STDERR, "ERROR! Wrong query\n");
						exit(EWQUERY);
					}
				}
				else{
					fprintf(STDERR, "ERROR! Wrong query\n");
					exit(EWQUERY);
				}

				// Condition skeleton will be stored in this array
				$syntaxArray = array();

				// Cycle processing parts of the condition
				for($i = 0; $i < count($qarr["con"]); $i++){
					// Operators are just pushed into condition skeleton array and the cycle continues
					if(in_array($qarr["con"][$i], array('&&', '||', '!', '(', ')')) || $qarr["con"][$i] == ""){
						array_push($syntaxArray, $qarr["con"][$i]);
						continue;
					}

					// Something other than operator was found, condition checking and translation begins
					array_push($syntaxArray, 'c');
					if(count($qarr["con"]) <= $i+2){
						fprintf(STDERR, "ERROR! Wrong query\n");
						exit(EWQUERY);
					}

					// Verifies the validity of element's name
					if(preg_replace("/^[.a-zA-Z_][\w.\-]+/", '', $qarr["con"][$i]) !== ''){
						fprintf(STDERR, "ERROR! Wrong query\n");
						exit(EWQUERY);
					}

					// Checks if element's name is not a keyword
					if(in_array($qarr["con"][$i], array('SELECT', 'LIMIT', 'FROM', 'ROOT', 'NOT', 'AND',
					                          'OR', 'CONTAINS', 'WHERE', 'ORDER', 'BY', 'ASC', 'DESC'))){
						fprintf(STDERR, "ERROR! Wrong query\n");
						exit(EWQUERY);
					}

					// Depending on an operator, different condition is created. Function getElemOrAttrVal
					// is used to retrieve value of the desired element or attribute during evaluation.
					switch ($qarr["con"][$i+1]) {
						case 'CONTAINS':
							// Checks whether value is a valid string
							if(!isProperlyQuoted($qarr["con"][$i+2])){
								fprintf(STDERR, "ERROR! Wrong query\n");
								exit(EWQUERY);
							}
							if($qarr["con"][$i+2] == '""')
								$qarr["con"][$i] = 'false';
							else 
								$qarr["con"][$i] = '(strpos((string) getElemOrAttrVal($element, "' .
								$qarr["con"][$i] . '"), (string)' .	$qarr["con"][$i+2] . ') !== FALSE)';
							break;
						case '>':
						case '<':
						case '==':
							if(!isProperlyQuoted($qarr["con"][$i+2]) && !isValidNumber($qarr["con"][$i+2])){
								fprintf(STDERR, "ERROR! Wrong query\n");
								exit(EWQUERY);
							}
							if($qarr["con"][$i+2] == '""')
								$qarr["con"][$i] = 'false';
							else 
								$qarr["con"][$i] = '(($tmp = getElemOrAttrVal($element, "'.$qarr["con"][$i].
								               '"))? $tmp '.$qarr["con"][$i+1].$qarr["con"][$i+2].': false)';
							break;
						default:
							// Unknown operator
							fprintf(STDERR, "ERROR! Wrong query\n");
							exit(EWQUERY);
					}
					$qarr["con"][$i+1] = $qarr["con"][$i+2] = "";
					$i += 2;
				}

				// Calling of the condition syntax check
				if(!checkConditionSyntax($syntaxArray)){
					fprintf(STDERR, "ERROR! Wrong query\n");
					exit(EWQUERY);
				}
				$qarr["con"] = implode(" ", $qarr["con"]);
			}
			else $qarr["con"] = 'true';
		}
	}



	/*!
	 * Selects elements matching the criteria of 'SELECT element FROM element-or-attribute' clause.
	 * @param args An array containing information about arguments, gained from parseCommLineArgs function.
	 * @param xml XML file loaded into a SimpleXMLElement variable.
	 * @return Returns an array containing desired elements.
	 */
	function selectFrom(&$args, &$xml){
		$filter;
		if($args["query"]["from"] != "ROOT"){

			if(preg_replace("/^[.a-zA-Z_][\w.\-]+/", '', $args["query"]["from"]) !== ''){
				fprintf(STDERR, "ERROR! Wrong query\n");
				exit(EWQUERY);
			}

			$args["query"]["from"] = explode(".", $args["query"]["from"]);
			
			switch(count($args["query"]["from"])){
				case 1:
					$filter = $xml->xpath("//". $args["query"]["from"][0] ."[1]");
					break;
				case 2:
					if($args["query"]["from"][1] == ""){
						fprintf(STDERR, "ERROR! Wrong query\n");
						exit(EWQUERY);
					}
					if($args["query"]["from"][0] == ""){
						$args["query"]["from"][0] = "*";
					}
					$filter = $xml->xpath("//".$args["query"]["from"][0]."[@".$args["query"]["from"][1]."][1]");
					break;
				default:
					fprintf(STDERR, "ERROR! Wrong query\n");
					exit(EWQUERY);
			}
			if(gettype($filter) == "array" and count($filter) != 0)
				$filter = $filter[0];
		}
		else $filter = $xml;

		if(preg_replace("/^[a-zA-Z_][\w\-]+/", '', $args["query"]["select"]) !== ''){
			fprintf(STDERR, "ERROR! Wrong query\n");
			exit(EWQUERY);
		}

		if(gettype($filter) != "array")
			$filter = $filter->xpath(".//". $args["query"]["select"]);

		return $filter;
	}



	/*!
	 * Callback function used for array filtering in condition parsing.
	 * @param string A string to be checked for not being whitespace
	 */
	function isNotSpace($string){
		return !ctype_space($string);
	}



	/*!
	 * Callback function used for array filtering in condition syntax checking.
	 * @param string A string to be checked for not being empty
	 */
	function isNotEmpty($string){
		return !empty($string);
	}



	/*!
	 * Function used for conditions in query translation.
	 * @param variable Variable to be checked if it is a valid number 
	 */
	function isValidNumber($variable){
		if(preg_replace("/^(\+|\-)?[0-9]+$/", '', $variable) !== '')
			return false;
		return true;
	}



	/*!
	 * Function used for conditions in query translation.
	 * @param variable Variable to be checked if it is properly quoted (a string) 
	 */
	function isProperlyQuoted($variable){
		return ($variable[0] == '"' && substr($variable, -1) == '"');
	}



	/*!
	 * Verifies the syntax of the query condition. During the condition parsing
	 * is a syntax array being created, which consists of the following symbols:
	 *   - c  = basic condition in 'element-or-attribute operator value' format
	 *   - !  = logical negation in condition
	 *   - (  = left parenthesis
	 *   - )  = right parenthesis
	 *   - && = logical AND operator
	 *   - || = logical OR operator
	 * @param condition Array containing the skeleton of the query condition.
	 * @return true if valid condition skeleton was passed, otherwise false.
	 */
	function checkConditionSyntax($condition){
		if($condition[count($condition)-1] == "")
			$condition = array_slice($condition, 0, count($condition)-1);
		for($i = 0; $i < count($condition); $i++){
			if($condition[$i] == '!'){
				if($i != 0 && ($condition[$i-1] == ')' || $condition[$i-1] == 'c'))
					return false;
				elseif($i != count($condition)-1 && in_array($condition[$i+1], array(')','&&','||')))
					return false;
				else $condition[$i] = "";
			}
		}
		$condition = array_filter($condition, 'isNotEmpty');
		$condition = array_values($condition);

		for($i = 0; $i < count($condition); $i++){
			if($condition[$i] == '('){
				if($i != 0 && $condition[$i-1] == 'c')
					return false;
				elseif($i != count($condition)-1 && ($condition[$i+1] == '&&' || $condition[$i+1] == '||' || $condition[$i+1] == ')'))
					return false;
				else{
					$j = $i+1;
					for(; $j < count($condition); $j++){
						if($condition[$j] == ')'){
							if($condition[$j-1] == '&&' || $condition[$j-1] == '||')
								return false;
							elseif($j != count($condition)-1 && ($condition[$j+1] == 'c' || $condition[$j+1] == '('))
								return false;
							else{
								$condition[$i] = $condition[$j] = "";
								break;
							}
						}
					}
					if($j == count($condition))
						return false;
				}
			}
			elseif($condition[$i] == ')')
				return false;
		}
		$condition = array_filter($condition, 'isNotEmpty');
		$condition = array_values($condition);

		for($i = 0; $i < count($condition); $i++){
			if(($i == 0 || $i == count($condition)-1) && $condition[$i] != 'c')
				return false;
			elseif($i % 2 == 1 && $condition[$i] == 'c')
				return false;
		}
		return true;
	}



	/*!
	 * Returns the value of an element or an attribute in another element. Used for
	 * condition evaluation and for element sorting.
	 * @param element SimpleXMLElement, in which is search performed.
	 * @param name Name of an element and/or an attribute, which value should be returned.
	 * @return String containing the value of the desired element, or false if element was not found.
	 */
	function getElemOrAttrVal(&$element, $name){
		$value;
		$parts = explode(".", $name);
			
		switch(count($parts)){
			case 1:
				$value = $element->xpath(".//". $parts[0] ."[1]");
				break;
			case 2:
				if($parts[1] == ""){
					fprintf(STDERR, "ERROR! Wrong query\n");
					exit(EWQUERY);
				}
				if($parts[0] == "")
					$parts[0] = "*";
				

				if($parts[0] == $element->getName() || $parts[0] == "*"){
					if($element->attributes()->$parts[1] != false){
						$value = $element;
						break;
					}
				}

				$value = $element->xpath(".//". $parts[0] ."[@" . $parts[1] . "][1]");
				break;
			default:
				fprintf(STDERR, "ERROR! Wrong query\n");
				exit(EWQUERY);
		}

		if(count($value) != 0)
			$value = $value[0];
		else return false;

		if(count($parts) == 1){
			if($value->count() != 0){
				fprintf(STDERR, "ERROR! Input is in an an illegal format!\n");
				exit(EINPUTF);
			}
			else return $value[0];
		}
		else return $value->attributes()->$parts[1];
	}



	/*!
	 * Callback function used for element sorting. Name of an element and/or
	 * an attribute and order, depending on which array should be sorted, is
	 * saved in the arguments array.
	 * @param a First element for comparison .
	 * @param b Second element for comparison.
	 */
	function cmpElemValues($a, $b){
		global $args;
		$aval = getElemOrAttrVal($a, $args["query"]["ord"][0]);
		$bval = getElemOrAttrVal($b, $args["query"]["ord"][0]);
		if($aval == false || $bval == false){
			fprintf(STDERR, "ERROR! Order element and/or attribute is missing!\n");
			exit(EINPUTF);
		}
		$i = strnatcmp($aval, $bval);
		return $args["query"]["ord"][1] == 'ASC'? $i : $i*(-1);
	}
?>
