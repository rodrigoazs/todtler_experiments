package ranker;

import java.io.File;
import java.io.FilenameFilter;
import java.io.IOException;
import java.text.DecimalFormat;
import java.text.NumberFormat;
import java.util.ArrayList;
import java.util.HashMap;
import java.util.List;
import java.util.Map;

import org.apache.commons.io.FileUtils;

import ranker.formula.Formula;
import ranker.instance.Instance;
import ranker.settings.Settings;
import ranker.template.Template;
import ranker.template.TemplateType;
import ranker.util.NameMapping;

import com.beust.jcommander.JCommander;
import com.beust.jcommander.ParameterException;

/**
 * @author Jan Van Haaren
 * @version 1.0
 */
public class RankingBuilder {

	private static NumberFormat NUMBER_FORMAT = new DecimalFormat("0.000");

	public static void main(String[] arguments) {

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

		// retrieve files
		File directory = settings.getDirectory();
		File templateInputFile = settings.getTemplateInputFile();
		File formulaInputFile = settings.getFormulaInputFile();

		// construct ranking
		Ranking ranking = getRanking(directory, templateInputFile, formulaInputFile);

		// normalize ranking
		ranking.normalize();

		// write template ranking to file
		writeContentToFile(settings.getTemplateOutputFile(), buildTemplateRankingContent(ranking, templateInputFile, formulaInputFile));

		// write formula ranking to file
		writeContentToFile(settings.getFormulaOutputFile(), buildFormulaRankingContent(ranking));
	}

	public static Ranking getRanking(final File directory, final File templateInputFile, final File formulaInputFile) {
		File[] files = getFiles(directory, "log");
		NameMapping nameMapping = new NameMapping(templateInputFile, formulaInputFile);
		List<Template> templates = new ArrayList<Template>();
		Map<String, Template> templateMapping = new HashMap<String, Template>();
		for (File file : files) {
			boolean containsConstants = file.getName().contains("constant");
			int templateId = getTemplateId(file.getName());
			String suffix = containsConstants ? "-constant" : "";
			String templateKey = templateId + suffix;
			if (!templateMapping.containsKey(templateKey)) {
				Template template = new Template(templateId, containsConstants, nameMapping.getTemplateMapping(templateId));
				templateMapping.put(templateKey, template);
				templates.add(template);
			}
			Template template = templateMapping.get(templateKey);
			Formula formula = getFormula(file, template, nameMapping);
			template.addFormula(formula);
		}
		Ranking ranking = new Ranking();
		ranking.addTemplates(templates);
		return ranking;
	}

	private static List<String> buildTemplateRankingContent(Ranking ranking, final File templateInputFile, final File formulaInputFile) {
		List<String> content = new ArrayList<String>();
		NameMapping nameMapping = new NameMapping(templateInputFile, formulaInputFile);
		for (Template template : ranking.getTemplates(TemplateType.ALL)) {
			boolean constants = template.containsConstants();
			boolean isDefinite = template.getFormulas().get(0).getInstances().get(0).isDefinite();
			if (constants || !isDefinite) {
				continue;
			}
			content.add("\t" + NUMBER_FORMAT.format(template.getScore()) + "\t" + nameMapping.getTemplateMapping(template.getTemplateId()));
			content.add("\t\t");
			List<Formula> formulas = template.getFormulas();
			for (Formula formula : formulas) {
				List<Instance> instances = formula.getInstances();
				if (instances.size() == 1) {
					content.add("\t\t\t" + NUMBER_FORMAT.format(formula.getScore()) + " " + instances.get(0).getInstance());
				}
				else {
					content.add("\t\t\t" + NUMBER_FORMAT.format(formula.getScore()) + " " + nameMapping.getFormulaMapping(formula.getFormulaId(), formula.getTemplate().containsConstants()));
					content.add("\t\t\t\t");
					for (Instance instance : instances) {
						content.add("\t\t\t\t\t" + NUMBER_FORMAT.format(instance.getScore()) + " " + instance.getInstance());
					}
					content.add("\t\t\t\t");
					content.add("\t\t\t");
				}
			}
			content.add("\t\t");
			content.add("\t");
		}
		return content;
	}

	private static List<String> buildFormulaRankingContent(Ranking ranking) {
		List<String> content = new ArrayList<String>();
		for (Instance instance : ranking.getInstances()) {
			content.add(NUMBER_FORMAT.format(instance.getScore()) + "\t" + instance.getInstance() + "\t" + instance.getFormula().getTemplate().getMultiplier());
		}
		return content;
	}

	private static Formula getFormula(final File file, final Template template, final NameMapping nameMapping) {
		int formulaId = getFormulaId(file.getName());
		List<String> content = readContentFromFile(file);
		Formula formula = new Formula(template, formulaId, nameMapping.getFormulaMapping(formulaId, file.getName().contains("constant")));
		boolean save = false;
		for (String line : content) {
			if (line.startsWith("evaluating gain of candidates took")) {
				save = !save;
			}
			if (save) {
				String score = line.substring(0, line.indexOf(" "));
				String instance = line.replaceAll(score, "").trim();
				formula.addInstance(new Instance(formula, Double.valueOf(score), instance));
			}
			if (line.startsWith("evaluating gain of candidates...")) {
				save = !save;
			}
		}
		return formula;
	}

	private static int getTemplateId(String fileName) {
		return Integer.valueOf(fileName.replaceAll("-output.log", "").split("-form")[0].replaceAll("temp-", ""));
	}

	private static int getFormulaId(String fileName) {
		return Integer.valueOf(fileName.replaceAll("-output.log", "").split("-form-")[1].replaceAll("-constant", ""));
	}

	private static File[] getFiles(final File directory, final String extension) {
		FilenameFilter filter = new FilenameFilter() {
			public boolean accept(File directory, String fileName) {
				return fileName.endsWith("." + extension);
			}
		};
		return directory.listFiles(filter);
	}

	private static List<String> readContentFromFile(File file) {
		List<String> content = new ArrayList<String>();
		try {
			content = FileUtils.readLines(file);
		}
		catch (IOException e) {
			e.printStackTrace();
		}
		return content;
	}

	private static void writeContentToFile(File outputFile, List<String> content) {
		try {
			FileUtils.writeLines(outputFile, content);
		}
		catch (IOException e) {
			e.printStackTrace();
		}
	}

}
