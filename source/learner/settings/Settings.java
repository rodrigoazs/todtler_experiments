package learner.settings;

import java.io.File;
import java.util.ArrayList;
import java.util.List;

import learner.LearnStrategy;
import ranker.MergeOperator;
import ranker.template.TemplateType;

import com.beust.jcommander.Parameter;

/**
 * @author Jan Van Haaren
 * @version 1.0
 */
public class Settings {

	@Parameter(names = "-domainFile", description = "Path to the domain input MLN file.", required = true)
	private String domainFile = "";

	@Parameter(names = "-templateFileSource", description = "Path to the template input CSV file for the source domain.", required = true)
	private String templateFileSource = "";

	@Parameter(names = "-formulaFileSource", description = "Path to the formula input CSV file for the source domain.", required = true)
	private String formulaFileSource = "";

	@Parameter(names = "-sourceDirectory", description = "Path to the directory for the source domain.", required = true)
	private String sourceDirectory = "";

	@Parameter(names = "-templateFileTarget", description = "Path to the template input CSV file for the target domain.", required = true)
	private String templateFileTarget = "";

	@Parameter(names = "-formulaFileTarget", description = "Path to the formula input CSV file for the target domain.", required = true)
	private String formulaFileTarget = "";

	@Parameter(names = "-targetDirectory", description = "Path to the directory for the target domain.", required = true)
	private String targetDirectory = "";

	@Parameter(names = "-outputDirectory", description = "Path to the output directory.", required = true)
	private String outputDirectory = "";

	@Parameter(names = "-train", description = "Training databases", required = true)
	private List<String> trainingDatabases = new ArrayList<String>();

	@Parameter(names = "-merge", description = "Merge operator", required = false, validateWith = MergeOperatorValidator.class)
	private String mergeOperator = "multiply";

	@Parameter(names = "-strategy", description = "Learn strategy", required = false, validateWith = LearnStrategyValidator.class)
	private String learnStrategy = "formula-greedy";

	@Parameter(names = "-templateType", description = "Type of templates to consider", required = false, validateWith = TemplateTypeValidator.class)
	private String templateType = "all";

	@Parameter(names = "-ne", description = "Non evidence", required = true)
	private List<String> nonEvidencePredicates = new ArrayList<String>();

	@Parameter(names = "-minWt", description = "Minimum clause weight", required = false)
	private double minimumWeight = 0.05;

	@Parameter(names = "-penalty", description = "Complexity penalty", required = false)
	private double complexityPenalty = 0.001;

	@Parameter(names = "-fractAtoms", description = "Fraction of ground atoms to draw", required = false)
	private double fractionOfAtoms = 1;

	@Parameter(names = "-maxAtomSamples", description = "Maximum number of atoms to sample", required = false)
	private int maximumAtomSamples = 10000;

	@Parameter(names = "-maxClauseSamples", description = "Maximum number of clauses to sample", required = false)
	private int maximumClauseSamples = 10000;

	@Parameter(names = "-tightConvThresh", description = "Tight convergence threshold", required = false)
	private double tightConvergenceThreshold = 0.001;

	@Parameter(names = "-looseConvThresh", description = "Loose convergence threshold", required = false)
	private double looseConvergenceThreshold = 0.01;

	@Parameter(names = "-tightMaxIter", description = "Tight maximum number of iterations", required = false)
	private double tightMaximumNumberOfIterations = 10;

	@Parameter(names = "-looseMaxIter", description = "Loose maximum number of iterations", required = false)
	private double looseMaximumNumberOfIterations = 10;

	public File getDomainFile() {
		return new File(this.domainFile);
	}

	public File getTemplateFileSourceDomain() {
		return new File(this.templateFileSource);
	}

	public File getFormulaFileSourceDomain() {
		return new File(this.formulaFileSource);
	}

	public File getSourceDirectory() {
		return new File(this.sourceDirectory);
	}

	public File getTemplateFileTargetDomain() {
		return new File(this.templateFileTarget);
	}

	public File getFormulaFileTargetDomain() {
		return new File(this.formulaFileTarget);
	}

	public File getTargetDirectory() {
		return new File(this.targetDirectory);
	}

	public File getOutputDirectory() {
		return new File(this.outputDirectory);
	}

	public MergeOperator getMergeOperator() {
		return MergeOperator.valueOf(this.mergeOperator.toUpperCase());
	}

	public LearnStrategy getLearnStrategy() {
		return LearnStrategy.valueOf(this.learnStrategy.replaceAll("-", "_").toUpperCase());
	}

	public TemplateType getTemplateType() {
		return TemplateType.valueOf(this.templateType.replaceAll("-", "_").toUpperCase());
	}

	public List<File> getTrainingDatabases() {
		return this.convertFileNamesToFiles(this.trainingDatabases);
	}

	private List<File> convertFileNamesToFiles(List<String> fileNames) {
		List<File> result = new ArrayList<File>();
		for (String fileName : fileNames) {
			result.add(new File(fileName));
		}
		return result;
	}

	public List<String> getNonEvidencePredicates() {
		return this.nonEvidencePredicates;
	}

	public String getNonEvidencePredicatesAsString() {
		StringBuilder stringBuilder = new StringBuilder();
		String prefix = "";
		for (String nonEvidencePredicate : this.getNonEvidencePredicates()) {
			stringBuilder.append(prefix);
			stringBuilder.append(nonEvidencePredicate);
			prefix = ",";
		}
		return stringBuilder.toString();
	}

	public double getMinimumWeight() {
		return this.minimumWeight;
	}

	public double getComplexityPenalty() {
		return this.complexityPenalty;
	}

	public double getFractionOfAtoms() {
		return this.fractionOfAtoms;
	}

	public int getMaximumAtomSamples() {
		return this.maximumAtomSamples;
	}

	public int getMaximumClauseSamples() {
		return this.maximumClauseSamples;
	}

	public double getTightConvergenceThreshold() {
		return this.tightConvergenceThreshold;
	}

	public double getLooseConvergenceThreshold() {
		return this.looseConvergenceThreshold;
	}

	public double getTightMaximumNumberOfIterations() {
		return this.tightMaximumNumberOfIterations;
	}

	public double getLooseMaximumNumberOfIterations() {
		return this.looseMaximumNumberOfIterations;
	}

}