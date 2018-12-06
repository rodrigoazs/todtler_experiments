package learner;

import java.io.File;
import java.util.ArrayList;
import java.util.List;

import learner.settings.Settings;

import org.apache.commons.io.FileUtils;

import ranker.Ranking;
import ranker.formula.Formula;
import ranker.template.Template;
import util.OutputLogger;

/**
 * @author Jan Van Haaren
 * @version 1.0
 */
public enum LearnStrategy {

	FORMULA_GREEDY("formula-greedy") {
		@Override
		public void run(Ranking ranking, File experimentDirectory, File domainFile, List<File> databases, Settings settings, OutputLogger outputLogger) throws Exception {

			List<String> declarations = FileUtils.readLines(domainFile);
			List<Formula> candidateFormulas = ranking.getFormulas(settings.getTemplateType());
			List<Formula> theory = new ArrayList<Formula>();

			double score = -Double.MAX_VALUE;
			outputLogger.log("--------------------");
			outputLogger.log("Number of candidate formulas: " + candidateFormulas.size());
			for (int i = 0; i < candidateFormulas.size(); i++) {
				Formula candidateFormula = candidateFormulas.get(i);
				System.out.println("Formula " + i + ": " + candidateFormula.getRepresentation());
			}
			outputLogger.log("Initial score: " + score);
			outputLogger.log("--------------------");
			int modelCount = 0;
			for (int iteration = 0; iteration < candidateFormulas.size(); iteration++) {

				File inputFile = new File(experimentDirectory, "input-iteration" + iteration + ".mln");
				File outputFile = new File(experimentDirectory, "output-iteration" + iteration + ".mln");
				File logFile = new File(experimentDirectory, "output-iteration" + iteration + ".log");

				Formula formula = candidateFormulas.get(iteration);
				theory.add(formula);

				StructureLearner.writeCandidateTheoryToFile(declarations, theory, inputFile);

				long startTime = System.currentTimeMillis();
				double candidateScore = StructureLearner.runExperiment(inputFile, outputFile, databases, logFile, settings);
				long endTime = System.currentTimeMillis();
				long runTime = endTime - startTime;
				outputLogger.log("Candidate theory score is " + candidateScore + ".");

				if (candidateScore <= score) {
					outputLogger.log("Candidate theory is worse. Remove added formula.");
					theory.remove(formula);
				}
				else {
					outputLogger.log("Candidate theory is better. Keep added formula.");
					score = candidateScore;
					FileUtils.copyFile(outputFile, new File(experimentDirectory, "model-" + modelCount + ".mln"));
					modelCount++;
				}

				outputLogger.log("Score: " + score);
				outputLogger.log("Iteration runtime: " + (runTime / 1000) + " s");
				outputLogger.log("--------------------");
			}
		}
	},

	TEMPLATE_GREEDY("template-greedy") {
		@Override
		public void run(Ranking ranking, File experimentDirectory, File domainFile, List<File> databases, Settings settings, OutputLogger outputLogger) throws Exception {

			List<String> declarations = FileUtils.readLines(domainFile);
			List<Template> candidateTemplates = ranking.getTemplates(settings.getTemplateType());
			List<Formula> theory = new ArrayList<Formula>();

			double score = -Double.MAX_VALUE;
			outputLogger.log("--------------------");
			outputLogger.log("Number of candidate templates: " + candidateTemplates.size());
			outputLogger.log("Initial score: " + score);
			outputLogger.log("--------------------");
			int modelCount = 0;
			for (int iteration = 0; iteration < candidateTemplates.size(); iteration++) {

				File inputFile = new File(experimentDirectory, "input-iteration" + iteration + ".mln");
				File outputFile = new File(experimentDirectory, "output-iteration" + iteration + ".mln");
				File logFile = new File(experimentDirectory, "output-iteration" + iteration + ".log");

				List<Formula> formulas = candidateTemplates.get(iteration).getFormulas();
				theory.addAll(formulas);

				StructureLearner.writeCandidateTheoryToFile(declarations, theory, inputFile);

				long startTime = System.currentTimeMillis();
				double candidateScore = StructureLearner.runExperiment(inputFile, outputFile, databases, logFile, settings);
				long endTime = System.currentTimeMillis();
				long runTime = endTime - startTime;
				outputLogger.log("Candidate theory score is " + candidateScore + ".");

				if (candidateScore <= score) {
					outputLogger.log("Candidate theory is worse. Remove added formulas.");
					theory.removeAll(formulas);
				}
				else {
					outputLogger.log("Candidate theory is better. Keep added formulas.");
					score = candidateScore;
					FileUtils.copyFile(outputFile, new File(experimentDirectory, "model-" + modelCount + ".mln"));
					modelCount++;
				}

				outputLogger.log("Score: " + score);
				outputLogger.log("Iteration runtime: " + (runTime / 1000) + " s");
				outputLogger.log("--------------------");
			}
		}
	};

	public abstract void run(Ranking ranking, File experimentDirectory, File domainFile, List<File> databases, Settings settings, OutputLogger outputLogger) throws Exception;

	private LearnStrategy(final String name) {
		this.name = name;
	}

	public String getName() {
		return this.name;
	}

	private final String name;

}