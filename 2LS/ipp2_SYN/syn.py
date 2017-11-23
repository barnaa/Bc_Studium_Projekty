#!/usr/bin/python3

#SYN:xbarna01

import sys, codecs, re



##!
# Prints help message.
def printHelp():
	print('''Highlights elements in input file based on rules defined in format file.
Usage: ./syn.py <arguments>
Arguments: --help\t\tPrints this message.
	   --output=[filename]\tSpecifies input file. If not set, standard input is used.
	   --output=[filename]\tSpecifies output file. If not set, standard output is used.
	   --format=[filename]\tSpecifies format file. Behaves as if empty file was given, if not set.
	   --br\t\t\tInserts <br /> tag at the end of every line in the output file.''')



##!
# Parses arguments and returns them as a dictionary with corresponding value.
# @param argv An array with arguments - usually sys.argv
# @return Dictionary containing parsed arguments
def parseArgs(argv):
	args = {'input': None,'output': None, 'format': None, 'br': False, 'help': False, 'err': False}
	ar = argv
	del(ar[0])
	for arg in ar:
		if(arg == '--help'):
			if(not args['help']):
				args['help'] = True
			else:
				args['err'] = True
				break
		elif(arg.startswith('--input=')):
			if(not args['input']):
				args['input'] = arg[8:]
			else:
				args['err'] = True
				break
		elif(arg.startswith('--output=')):
			if(not args['output']):
				args['output'] = arg[9:]
			else:
				args['err'] = True
				break
		elif(arg.startswith('--format=')):
			if(not args['format']):
				args['format'] = arg[9:]
			else:
				args['err'] = True
				break
		elif(arg == '--br'):
			if(not args['br']):
				args['br'] = True
			else:
				args['err'] = True
				break
		else:
			args['err'] = True
			break
	if(args['help'] and (args['input'] or args['output'] or args['format'] or args['br'])):
		args['err'] = True
	if(args['err']):
		sys.stderr.write('Script was launched with an illegal combination of arguments!\n')
		#printHelp()
		exit(1)
	if(args['help']):
		printHelp()
		exit(0)
	return args



##!
# Finds indexes of opening and closing parentheses of negated parenthesised expression.
# Used in regular expression translation.
# @param s String, which contains negated parenthesised expression
# @return 2-Tuple of opening and closing parentheses' indexes.
def getNegGrBounds(s):
	start = re.search('!\(',s).start() + 1
	parLevel = 1
	index = start + 1
	while(parLevel):
		if(s[index] == ')' and s[index-1] != '\\'):
			parLevel -= 1
		elif(s[index] == '(' and s[index-1] != '\\'):
			parLevel += 1
		index += 1
	return (start, index)



##!
# Finds indexes of opening and closing parentheses of the most nested parenthesised
# expression. Used in regular expression translation.
# @param s String, which contains parenthesised expression
# @return 2-Tuple of opening and closing parentheses' indexes.
def getGroupBounds(s):
	matches = re.finditer(r'([^\\]|^)\([^()]+\)', s)
	m = next(matches, None)
	if(m):
		return (m.start(), m.end())
	else:
		return None



##!
# Transforms regex list in brackets into a python set.
# Used in regular expression translation.
# @param s String containing bracketed regex list, eg. [abc]
# @return Set transformed from regex list.
def getSetFromRegexList(s):
	out = set();
	x = 1
	if(s[1] == '^'):
		x = 2
	for i in range(x, len(s)-1):
		if(s[i] == '-'):
			for l in range(ord(s[i-1])+1, ord(s[i+1])):
				out.add(chr(l))
		elif(s[i] == '\\'):
			if(s[i+1] == 'd'):
				for d in range(0, 10):
					out.add(str(d))
			elif(s[i+1] == 's'):
				out.add(' ')
				out.add('\\t')
				out.add('\\n')
				out.add('\\r')
				out.add('\\f')
				out.add('\\v')
			else:
				out.add('\\' + s[i+1])
		elif(i > 0 and (s[i] == 'd' or s[i] == 's') and s[i-1] == '\\'):
			continue
		else:
			out.add(s[i])
	return out



##!
# Transforms parenthesised expression into a regex list. Parenthesised expression
# must consist of expressions matching single symbols, divided by alternation operators.
# Used in regular expression translation.
# @param s String, which contains parenthesised expression
# @param b Index, at which the parenthesised expression begins
# @return Regex list as string, or '.' if parenthesised expression matches any character, or "/0" if it matches no character.
def getRegexListFromGroup(s, b):
	group = s[b[0]+1:b[1]-1]
	gp = group.split('|')
	for i in range(len(gp)):
		if(len(gp[i]) > 2):
			if(re.fullmatch(r'!?\[.*\]', gp[i])):
				if(gp[i][0] == '!'):
					if(gp[i][2] == '^'):
						gp[i] = gp[i][1] + gp[i][3:]
						getSetFromRegexList(gp[i])
					else:
						gp[i] = gp[i][1] + '^' + gp[i][2:]
						getSetFromRegexList(gp[i])
			elif(re.fullmatch(r'!\\(\^|\$|\?|\{|\}|\[|\]|s|d|t|n|\*|\+|\(|\)|0)', gp[i])):
				gp[i] = '[^' + gp[i][1:] + ']'
			else:
				raise Exception('Wrong pattern')
		elif(len(gp[i]) == 2):
			if(re.fullmatch(r'!.', gp[i])):
				if(gp[i][1] == '.'):
					gp[i] = '\\0'
				else:
					gp[i] = '[^' + gp[i][1] + ']'
			elif(re.fullmatch(r'(\\(\^|\$|\?|\{|\}|\[|\]|s|d|t|n|\*|\+|\(|\)|0))', gp[i])):
				gp[i] = '[' + gp[i] + ']'
			else:
				raise Exception('Wrong pattern')
		elif(len(gp[i]) == 1):
			if(gp[i] == '.'):
				return '.'
			gp[i] = '[' + gp[i] + ']'
		else:
			raise Exception('Wrong pattern')

	neg = False
	cset = set()
	for sid in range(len(gp)):
		if(gp[sid] == '\\0'):
			continue
		if(neg):
			if(gp[sid][1] == '^'):
				cset &= getSetFromRegexList(gp[sid])
			else:
				cset -= getSetFromRegexList(gp[sid])
		else:
			if(gp[sid][1] == '^'):
				neg = True
				cset = getSetFromRegexList(gp[sid]) - cset
			else:
				cset |= getSetFromRegexList(gp[sid])
		if(not len(cset)):
			return '.'
	if(neg):
		out = '[^'
	else:
		out = '['

	for l in cset:
		out += l
	out += ']'

	if(out == '[]'):
		return '\\0'
	else:
		return out



##!
# Transforms string containing negated parenthesised expression.
# Used in regular expression translation.
# @param s String, which contains negated parenthesised expression.
# @return Transformed string.
def resolveNegatedGroup(s):
	bds = getNegGrBounds(s)
	se = s[bds[0]:bds[1]]
	grb = getGroupBounds(se)
	while grb:
		se = se[:grb[0]] + getRegexListFromGroup(se, grb) + se[grb[1]:]
		grb = getGroupBounds(se)
	return s[:bds[0]] + se + s[bds[1]:]



##!
# Resolves successive quantifiers.
# @param reg Regular expression.
# @return Regular expression without duplicate quantifiers.
def nqsResolve(reg):
	reg = re.sub(r'\++','+', reg)
	reg = re.sub(r'(\+*\*+\+*)+','*', reg)
	return reg



##!
# Translates highlighting expression from format file into a Python regular expression.
# @param reg Expression from format file.
# @return Corresponding python regular expression.
def translateRegex(reg):
	reg = reg.replace('%%','\x11')
	reg = reg.replace('%!','\x12')
	reg = reg.replace('%.','\x13')
	reg = reg.replace('%|','\x14')

	reg = nqsResolve(reg)

	if(re.search('^[.|*+]', reg)):
		raise Exception('Wrong pattern')
	if(re.search('[!.%|]$', reg)):
		raise Exception('Wrong pattern')
	if(re.search('([^%]|^)[.|!(][*+.|)]', reg)):
		raise Exception('Wrong pattern')
	
	reg = reg.replace('.','')
	reg = reg.replace('\\','\\\\')
	reg = re.sub(r'(\^|\$|\?|\{|\}|\[|\])', r'\\\1', reg)

	reg = reg.replace('%a','.')
	reg = reg.replace('%s','\\s')
	reg = reg.replace('%d','\\d')
	reg = reg.replace('%l','[a-z]')
	reg = reg.replace('%L','[A-Z]')
	reg = reg.replace('%w','[a-zA-Z]')
	reg = reg.replace('%W','[a-zA-Z\\d]')

	reg = re.sub(r'%(t|n|\*|\+|\(|\))', r'\\\1', reg)
	if(reg.find('%') != -1):
		raise Exception('Wrong pattern')

	reg = reg.replace('!!','')
	if(re.search('!\(', reg)):
		reg = resolveNegatedGroup(reg)
	reg = reg.replace('!.','\\0')
	reg = re.sub(r'!\\(.)', r'[^\\\1]', reg)
	reg = re.sub(r'!([^\[])', r'[^\1]', reg)
	reg = re.sub('!\[^', '[', reg)
	reg = re.sub('!\[', '[^', reg)
	
	reg = reg.replace('\x11','%')
	reg = reg.replace('\x12','!')
	reg = reg.replace('\x13','\\.')
	reg = reg.replace('\x14','\\|')
	re.compile(reg)
	return reg



##!
# Creates a list of rules from format file.
# @param formatf Name of the file containing formatting information.
# @return List of rules in format (expression, (tag1,tag_parameter), (tag2,tag_parameter), ...).
def getRules(formatf):
	rules = list()
	try:
		ffile = codecs.open(formatf, 'r', 'utf-8')
	except:
		return None

	try:
		for line in ffile:
			if(line == '\n'):
				continue
			elif(line.rstrip() == ''):
				exit(4)
			r = re.split('\t+', line)
			tags = line[line.find(r[1].rstrip()):]
			tags = tags.rstrip()
			r = r[0]
			r = translateRegex(r)
			tags = re.split(',[ \t]*', tags)
			rule = (r,)
			for tag in tags:
				if tag == 'bold':
					rule = rule + (('b',),)
				elif tag == 'italic':
					rule = rule + (('i',),)
				elif tag == 'underline':
					rule = rule + (('u',),)
				elif tag == 'teletype':
					rule = rule + (('tt',),)
				else:
					tparts = tag.split(':')
					if(len(tparts) != 2):
						raise Exception('Invalid tag')
					elif(tparts[0] == 'color'):
						if(not re.fullmatch('[0-9a-fA-F]{6}', tparts[1])):
							raise Exception('Bad color')
						else:
							rule = rule + (('c',tparts[1]),)
					elif(tparts[0] == 'size'):
						if(int(tparts[1]) < 1 or int(tparts[1]) > 7):
							raise Exception('Bad size')
						else:
							rule = rule + (('s',tparts[1]),)
					else:
						raise Exception('Invalid tag')
			rules += (rule,)
	except:
		sys.stderr.write('Error in format file!\n')
		exit(4)

	ffile.close()
	
	return rules



##!
# Loads input file.
# @param fileName Name of the input file, None for standard input.
# @return Content of the input file, in string.
def getInput(fileName):
	if(fileName):
		try:
			inFile = codecs.open(fileName, 'r', 'utf-8')
		except:
			sys.stderr.write('An error occured while opening input file!\n')
			exit(2)
	else:
		inFile = sys.stdin
	
	fileCont = inFile.read()
	
	if(inFile is not sys.stdin):
		inFile.close()
	return fileCont



##!
# Opens output file.
# @param fileName Name of the output file, None for standard output.
# @return File descriptor of the output file.
def openOutputFile(fileName):
	if(fileName):
		try:
			outFile = codecs.open(fileName, 'w', 'utf-8')
		except:
			sys.stderr.write('An error occured while opening output file!\n')
			exit(3)
	else:
		outFile = sys.stdout
	return outFile



##!
# Finds matches for all rules.
# @param ifile Content of the input file, as a string.
# @param rules List of rules, returned from getRules() function.
# @return Lists of matches for every rule, in a list.
def findMatches(ifile, rules):
	matchList = list()
	for rule in rules:
		ruleMatchList = list()
		matches = re.finditer(rule[0], ifile, re.DOTALL)
		for match in matches:
			ruleMatchList += (match.start(),match.end()),
		matchList += ruleMatchList,
	return matchList



##!
# Creates opening and ending tags for a rule.
# @param rule Rule from the list of rules
# @return 2-Tuple of opening and closing tags for a rule.
def createTags(rule):
	rule = rule[1:]
	stag = etag = ''
	for tag in rule:
		if(tag[0] == 'b'):
			stag += '<b>'
			etag = '</b>' + etag
		elif(tag[0] == 'i'):
			stag += '<i>'
			etag = '</i>' + etag
		elif(tag[0] == 'u'):
			stag += '<u>'
			etag = '</u>' + etag
		elif(tag[0] == 'tt'):
			stag += '<tt>'
			etag = '</tt>' + etag
		elif(tag[0] == 'c'):
			stag += '<font color=#' + tag[1] + '>'
			etag = '</font>' + etag
		elif(tag[0] == 's'):
			stag += '<font size=' + tag[1] + '>'
			etag = '</font>' + etag
		else: 
			sys.stderr.write('Unknown tag in the rule!\n')
			exit(666)
	return (stag,etag,)



def main():
	# Parses arguments and loads desired files
	args = parseArgs(sys.argv)
	rules = getRules(args['format'])
	file = getInput(args['input'])
	outFile = openOutputFile(args['output'])

	# Inserts tags into input file, based on formatting rules
	if(rules):
		# Finds all matches
		matchList = findMatches(file, rules)
		# Inserts tags for every rule
		for r in range(len(rules)):
			tags = createTags(rules[r])
			# Inserts tags for every match
			for match in reversed(range(len(matchList[r]))):
				# Match of length 0
				if(matchList[r][match][0] == matchList[r][match][1]):
					continue
				# First inserts ending tag, then opening tag
				for mi in reversed(range(0, 2)):
					file = file[:matchList[r][match][mi]] + tags[mi] + file[matchList[r][match][mi]:]
					# Pushes further matches of remaining rules after inserted tag, so their indexes remain consistent in the edited file
					for rEd in range(r+1, len(rules)):
						for matchEd in reversed(range(len(matchList[rEd]))):
							if(matchList[rEd][matchEd][0] >= matchList[r][match][mi]):
								matchList[rEd][matchEd] = (matchList[rEd][matchEd][0] + len(tags[mi]), matchList[rEd][matchEd][1] + len(tags[mi]))
							elif(matchList[rEd][matchEd][1] > matchList[r][match][mi]):
								matchList[rEd][matchEd] = (matchList[rEd][matchEd][0], matchList[rEd][matchEd][1] + len(tags[mi]))
							else:
								break
	if(args['br']):
		file = file.replace('\n', '<br />\n')

	# Writes output into a file and closes it
	outFile.write(file)
	if(outFile != sys.stdout):
		outFile.close()
	exit(0)

main()
