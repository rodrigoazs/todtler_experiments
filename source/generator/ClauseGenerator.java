package generator;

import generator.formula.Formula;
import generator.formula.FormulaLiteral;
import generator.settings.Settings;
import generator.template.Template;
import generator.template.TemplateComparator;
import generator.template.TemplateLiteral;

import java.io.File;
import java.io.IOException;
import java.util.ArrayList;
import java.util.Arrays;
import java.util.Collections;
import java.util.HashMap;
import java.util.HashSet;
import java.util.List;
import java.util.Map;
import java.util.Set;

import org.apache.commons.io.FileUtils;
import org.paukov.combinatorics.Factory;
import org.paukov.combinatorics.Generator;
import org.paukov.combinatorics.ICombinatoricsVector;

import util.SortedList;

import com.beust.jcommander.JCommander;
import com.beust.jcommander.ParameterException;

/**
 * @author Jan Van Haaren
 * @version 1.0
 * 
 *          This class represents the clause generator.
 */
public class ClauseGenerator {

	public static void main(String[] arguments) throws Exception {

		// parse the settings
		Settings settings = new Settings();
		JCommander commander = new JCommander(settings);

		try {
			commander.parse(arguments);
		}
		catch (ParameterException e) {
			System.err.println(e.getMessage());
			commander.usage();
			System.exit(0);
		}

		// read all predicates and types from the domain file
		final Map<String, List<Type>> domainTypes = readTypesFromFile(settings.getDomainFile());

		// generate all valid template literals
		final List<TemplateLiteral> templateLiterals = generateTemplateLiterals(settings.getMaximumNumberOfLiterals(), settings.getMaximumNumberOfVariables());

		// construct maps
		final Map<Integer, String> templatesMap = new HashMap<Integer, String>();
		final Map<String, String> formulasMap = new HashMap<String, String>();
		final Map<String, Integer> mapping = new HashMap<String, Integer>();

		// generate templates and formulas
		generate(settings, domainTypes, templateLiterals, templatesMap, formulasMap, mapping);

		// write templates and formulas to file
		writeFormulasToFile(settings, formulasMap, mapping);
		writeFormulasOverviewToFile(settings, formulasMap);
		writeTemplatesOverviewToFile(settings, templatesMap);
	}

	private static void generate(Settings settings, final Map<String, List<Type>> domainTypes, final List<TemplateLiteral> templateLiterals, final Map<Integer, String> templateMapping, final Map<String, String> formulaMapping, final Map<String, Integer> mapping) throws IOException {
		// generate second-order templates of increasing length
		for (int length = settings.getMinimumNumberOfLiterals(); length <= settings.getMaximumNumberOfLiterals(); length++) {
			List<Template> templates = generateSecondOrderTemplates(templateLiterals, length);
			Collections.sort(templates, TemplateComparator.getInstance());
			// generate first-order formulas that are instantiations of the second-order templates
			for (Template template : templates) {
				int templateId = template.hashCode();
				templateMapping.put(templateId, template.toString());
				List<Formula> formulas = generateFirstOrderFormulas(template, domainTypes, settings.includeConstants());
				for (Formula formula : formulas) {
					String suffix = formula.isConstantVersion() ? "-constant" : "";
					String formulaId = formula.getIdentifier() + suffix;
					formulaMapping.put(formulaId, formula.toString());
					mapping.put(formulaId, templateId);
				}
			}
		}
	}

	public static void writeFormulasToFile(Settings settings, Map<String, String> formulaMapping, Map<String, Integer> mapping) throws Exception {
		List<String> formulaStrings = FileUtils.readLines(settings.getDomainFile());
		formulaStrings.add("");
		for (String formulaId : formulaMapping.keySet()) {
			List<String> model = new ArrayList<String>(formulaStrings);
			int templateId = mapping.get(formulaId);
			String formula = formulaMapping.get(formulaId);
			model.add("0 " + formula);
			String filename = "temp-" + templateId + "-form-" + formulaId + ".mln";
			File file = new File(settings.getFormulaDirectory(), filename);
			FileUtils.writeLines(file, model);
		}
	}

	public static void writeFormulasOverviewToFile(Settings settings, final Map<String, String> formulaMapping) throws IOException {
		System.out.println("Number of formulas: " + formulaMapping.size());
		List<String> formulaStrings = new ArrayList<String>();
		for (String key : formulaMapping.keySet()) {
			String formula = formulaMapping.get(key);
			formulaStrings.add(key + "\t" + formula);
		}
		FileUtils.writeLines(settings.getFormulaFile(), formulaStrings);
	}

	public static void writeTemplatesOverviewToFile(Settings settings, final Map<Integer, String> templateMapping) throws IOException {
		System.out.println("Number of templates: " + templateMapping.size());
		List<String> templateStrings = new ArrayList<String>();
		for (int key : templateMapping.keySet()) {
			templateStrings.add(key + "\t" + templateMapping.get(key));
		}
		FileUtils.writeLines(settings.getTemplateFile(), templateStrings);
	}

	private static List<TemplateLiteral> generateTemplateLiterals(final int numberOfLiterals, final int numberOfVariables) {
		List<TemplateLiteral> templateLiterals = new ArrayList<TemplateLiteral>();
		for (int i = 0; i < numberOfLiterals; i++) {
			for (int j = 0; j < numberOfVariables; j++) {
				for (int k = 0; k < numberOfVariables; k++) {
					templateLiterals.add(new TemplateLiteral(i, j, k));
				}
			}
		}
		return templateLiterals;
	}

	private static List<Template> generateSecondOrderTemplates(List<TemplateLiteral> templateLiterals, int numberOfLiterals) {

		// generate templates
		ICombinatoricsVector<TemplateLiteral> initialVector = Factory.createVector(templateLiterals);
		Generator<TemplateLiteral> generator = Factory.createSimpleCombinationGenerator(initialVector, numberOfLiterals);

		// normalize templates
		List<Template> templates = new SortedList<Template>(TemplateComparator.getInstance());
		for (ICombinatoricsVector<TemplateLiteral> combination : generator) {
			List<TemplateLiteral> templateBlocks = combination.getVector();
			Template template = new Template();
			for (TemplateLiteral templateBlock : templateBlocks) {
				template.addLiteral(templateBlock);
			}
			template.normalize();
			if (!templates.contains(template)) {
				templates.add(template);
			}
		}

		// prune templates
		List<Template> prunedTemplates = pruneTemplates(templates);

		// negate templates
		List<Template> negatedTemplates = new SortedList<Template>(TemplateComparator.getInstance());
		for (Template template : prunedTemplates) {
			ICombinatoricsVector<Boolean> originalVector = Factory.createVector(new Boolean[] { true, false });
			Generator<Boolean> signGenerator = Factory.createPermutationWithRepetitionGenerator(originalVector, template.getLength());
			for (ICombinatoricsVector<Boolean> permute : signGenerator) {
				Template newTemplate = new Template(template);
				List<Boolean> truthValues = permute.getVector();
				for (int i = 0; i < truthValues.size(); i++) {
					if (!truthValues.get(i)) {
						newTemplate.getLiterals().get(i).negate();
					}
				}
				negatedTemplates.add(newTemplate);
			}
		}

		// prune negated templates
		return pruneTemplates(negatedTemplates);
	}

	private static List<Formula> generateFirstOrderFormulas(Template template, Map<String, List<Type>> types, boolean includeConstants) {
		List<Formula> formulas = new ArrayList<Formula>();
		Set<Integer> predicates = new HashSet<Integer>();
		List<Integer> arguments = new ArrayList<Integer>();
		for (TemplateLiteral literal : template.getLiterals()) {
			predicates.add(literal.getLiteralNumber());
			arguments.add(literal.getArgument1Number());
			arguments.add(literal.getArgument2Number());
		}
		// retrieve predicates whose arguments can be constants
		Set<String> constantPredicates = new HashSet<String>();
		for (String predicate : types.keySet()) {
			List<Type> typesList = types.get(predicate);
			for (Type type : typesList) {
				if (type.canAppearAsConstant()) {
					constantPredicates.add(predicate);
				}
			}
		}
		ICombinatoricsVector<String> originalVector = Factory.createVector(types.keySet());
		Generator<String> generator = Factory.createSimpleCombinationGenerator(originalVector, predicates.size());
		for (ICombinatoricsVector<String> combination : generator) {
			Generator<String> permutationGenerator = Factory.createPermutationGenerator(combination);
			for (ICombinatoricsVector<String> permutation : permutationGenerator) {
				List<String> predicateNames = permutation.getVector();
				Formula formula = new Formula(template);
				for (FormulaLiteral literal : formula.getLiterals()) {
					String predicateName = predicateNames.get(literal.getLiteralNumber() - 1);
					List<String> argumentTypes = new ArrayList<String>();
					for (Type type : types.get(predicateName)) {
						argumentTypes.add(type.getName());
					}
					literal.setLiteralName(predicateName);
					literal.setArgument1Type(argumentTypes.get(0));
					literal.setArgument2Type(argumentTypes.get(1));
				}
				if (formula.isValid()) {
					formula.normalize();
					formulas.add(formula);
					Formula constantFormula = formula.getConstantVersion(constantPredicates, types);
					if (constantFormula != null) {
						formulas.add(constantFormula);
					}
				}
			}
		}
		return formulas;
	}

	private static List<Template> pruneTemplates(List<Template> templates) {
		List<Template> prunedTemplates = new SortedList<Template>(TemplateComparator.getInstance());
		for (Template template : templates) {
			boolean accept = true;
			// check for connectivity and self-references
			if (!template.isConnected() || template.refersToSelf()) {
				accept = false;
			}
			// check for equivalence
			for (Template prunedTemplate : prunedTemplates) {
				if (areEqual(template, prunedTemplate)) {
					accept = false;
				}
			}
			if (accept) {
				prunedTemplates.add(template);
			}
		}
		return prunedTemplates;
	}

	private static boolean areEqual(Template formula1, Template formula2) {
		return areEqual(convertFormulaToStrings(formula1), convertFormulaToStrings(formula2));
	}

	private static List<String> convertFormulaToStrings(Template template) {
		List<String> list = new ArrayList<String>();
		for (TemplateLiteral literal : template.getLiterals()) {
			list.add(literal.toString());
		}
		return list;
	}

	private static boolean areEqual(List<String> formula1, List<String> formula2) {

		// copy first formula
		List<String> benchmark = new ArrayList<String>();
		benchmark.addAll(formula1);
		for (int i = 0; i < benchmark.size(); i++) {
			benchmark.set(i, benchmark.get(i).replaceAll("R", "S"));
			benchmark.set(i, benchmark.get(i).replaceAll("T", "U"));
		}

		// get predicates from copied first formula
		Set<String> predicates1 = new HashSet<String>();
		Set<String> predicates1b = new HashSet<String>();
		Set<String> arguments1 = new HashSet<String>();
		for (String literal : benchmark) {
			predicates1.add(getPredicateWithSign(literal));
			predicates1b.add(getPredicateWithoutSign(literal));
			arguments1.addAll(getArguments(literal));
		}

		// get predicates from second formula
		Set<String> predicates2 = new HashSet<String>();
		Set<String> predicates2b = new HashSet<String>();
		Set<String> arguments2 = new HashSet<String>();
		for (String literal : formula2) {
			predicates2.add(getPredicateWithSign(literal));
			predicates2b.add(getPredicateWithoutSign(literal));
			arguments2.addAll(getArguments(literal));
		}
		List<String> predicates2List = new ArrayList<String>(predicates2b);
		List<String> arguments2List = new ArrayList<String>(arguments2);

		if (predicates1.size() != predicates2.size() || predicates1b.size() != predicates2b.size() || arguments1.size() != arguments2.size()) {
			return false;
		}

		// permute predicates
		ICombinatoricsVector<String> predicatesVector = Factory.createVector(predicates1b);
		Generator<String> generator = Factory.createPermutationGenerator(predicatesVector);
		for (ICombinatoricsVector<String> permutation : generator) {
			List<String> workingCopy1 = new ArrayList<String>();
			workingCopy1.addAll(formula2);

			for (int i = 0; i < permutation.getSize(); i++) {
				for (int j = 0; j < workingCopy1.size(); j++) {
					workingCopy1.set(j, workingCopy1.get(j).replaceAll(predicates2List.get(i), permutation.getValue(i)));
				}
			}

			// permute arguments
			ICombinatoricsVector<String> argumentsVector = Factory.createVector(arguments1);
			Generator<String> generator2 = Factory.createPermutationGenerator(argumentsVector);
			for (ICombinatoricsVector<String> permutation2 : generator2) {
				List<String> workingCopy2 = new ArrayList<String>();
				workingCopy2.addAll(workingCopy1);
				for (int k = 0; k < permutation2.getSize(); k++) {
					for (int l = 0; l < workingCopy2.size(); l++) {
						workingCopy2.set(l, workingCopy2.get(l).replaceAll(arguments2List.get(k), permutation2.getValue(k)));
					}
				}

				Collections.sort(workingCopy2);
				if (workingCopy2.equals(benchmark)) {
					return true;
				}
			}
		}

		return false;
	}

	private static String getPredicateWithoutSign(String literal) {
		return literal.substring(0, literal.indexOf("(")).replaceAll("!", "");
	}

	private static String getPredicateWithSign(String literal) {
		return literal.substring(0, literal.indexOf("("));
	}

	private static List<String> getArguments(String literal) {
		String literalWithoutPlus = literal.replaceAll("\\+", "");
		return Arrays.asList(literalWithoutPlus.substring(literalWithoutPlus.indexOf("(") + 1, literalWithoutPlus.indexOf(")")).split(","));
	}

	private static List<Type> getArgumentTypes(String literal) {
		List<Type> types = new ArrayList<Type>();
		String[] arguments = literal.substring(literal.indexOf("(") + 1, literal.indexOf(")")).split(",");
		for (String argument : arguments) {
			types.add(new Type(argument.replaceAll("\\+", ""), argument.contains("+")));
		}
		return types;
	}

	private static Map<String, List<Type>> readTypesFromFile(File file) throws Exception {
		Map<String, List<Type>> map = new HashMap<String, List<Type>>();
		List<String> content = FileUtils.readLines(file);
		for (String line : content) {
			map.put(getPredicateWithoutSign(line), getArgumentTypes(line));
		}
		return map;
	}

}
