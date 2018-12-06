package learner;

import java.io.File;
import java.io.IOException;
import java.util.ArrayList;
import java.util.List;

import learner.settings.Settings;

import org.apache.commons.io.FileUtils;

import ranker.Ranking;
import ranker.RankingBuilder;
import ranker.formula.Formula;
import util.OutputLogger;
import util.Util;

import com.beust.jcommander.JCommander;
import com.beust.jcommander.ParameterException;

/**
 * @author Jan Van Haaren
 * @version 1.0
 * 
 *          This class represents the structure learner.
 */
public class StructureLearner {

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

		// build rankings
		Ranking sourceRanking = RankingBuilder.getRanking(settings.getSourceDirectory(), settings.getTemplateFileSourceDomain(), settings.getFormulaFileSourceDomain());
		Ranking targetRanking = RankingBuilder.getRanking(settings.getTargetDirectory(), settings.getTemplateFileTargetDomain(), settings.getFormulaFileTargetDomain());

		// merge rankings
		Ranking ranking = targetRanking.merge(sourceRanking, settings.getMergeOperator());

		// run the algorithm
		run(ranking, settings);
	}

	private static void run(Ranking ranking, Settings settings) throws Exception {

		LearnStrategy learnStrategy = settings.getLearnStrategy();

		// create experiment logger
		File experimentLogFile = new File(settings.getOutputDirectory(), "output.log");
		OutputLogger outputLogger = new OutputLogger(experimentLogFile);

		long startTime = System.currentTimeMillis();
		learnStrategy.run(ranking, settings.getOutputDirectory(), settings.getDomainFile(), settings.getTrainingDatabases(), settings, outputLogger);
		long endTime = System.currentTimeMillis();
		long runTime = endTime - startTime;
		outputLogger.log("Total runtime: " + (runTime / 1000) + " s");
	}

	public static double runExperiment(File inputFile, File outputFile, List<File> databaseFiles, File logFile, Settings settings) {
		double result = Double.MAX_VALUE;
		try {
			String databaseString = Util.convertFilesToString(databaseFiles);
			boolean multipleDatabases = databaseFiles.size() > 1 ? true : false;

			ProcessBuilder processBuilder =
					new ProcessBuilder("learnstruct", "-i", inputFile.getAbsolutePath(), "-o", outputFile.getAbsolutePath(), "-t", databaseString, "-ne", settings.getNonEvidencePredicatesAsString(), "-startFromEmptyMLN", "false", "-multipleDatabases", String.valueOf(multipleDatabases), "-plusType", "2", "-search",
							"666", "-minWt", String.valueOf(settings.getMinimumWeight()), "-penalty", String.valueOf(settings.getComplexityPenalty()), "-fractAtoms", String.valueOf(settings.getFractionOfAtoms()), "-maxAtomSamples", String.valueOf(settings.getMaximumAtomSamples()), "-maxClauseSamples",
							String.valueOf(settings.getMaximumClauseSamples()), "-tightConvThresh", String.valueOf(settings.getTightConvergenceThreshold()), "-looseConvThresh", String.valueOf(settings.getLooseConvergenceThreshold()), "-tightMaxIter", String.valueOf(settings.getTightMaximumNumberOfIterations()),
							"-looseMaxIter", String.valueOf(settings.getLooseMaximumNumberOfIterations()));
			processBuilder.redirectErrorStream(true);
			processBuilder.redirectOutput(logFile);

			Process process = processBuilder.start();
			process.waitFor();
			result = retrievePseudoLikelihood(logFile);
		}
		catch (IOException | InterruptedException e) {
			e.printStackTrace();
		}
		return result;
	}

	public static void writeCandidateTheoryToFile(List<String> declarations, List<Formula> theory, File inputFile) throws Exception {
		List<String> content = new ArrayList<String>();
		content.addAll(declarations);
		content.add("");
		for (Formula formula : theory) {
			content.add("0 " + formula.getRepresentation());
		}
		FileUtils.writeLines(inputFile, content);
	}

	private static double retrievePseudoLikelihood(File logFile) {
		double result = Double.MAX_VALUE;
		try {
			List<String> content = FileUtils.readLines(logFile);
			for (String line : content) {
				if (line.startsWith("score = ")) {
					String[] fields = line.split("=");
					result = Double.valueOf(fields[1].trim());
				}
			}
		}
		catch (IOException e) {
			e.printStackTrace();
		}
		return result;
	}

}
