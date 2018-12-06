package generator.settings;

import java.io.File;

import com.beust.jcommander.Parameter;

/**
 * @author Jan Van Haaren
 * @version 1.0
 * 
 *          This class contains the settings of the generator.
 */
public class Settings {

	@Parameter(names = "-minLiterals", description = "Minimum number of literals.", required = false)
	private int minimumNumberOfLiterals = 2;

	@Parameter(names = "-maxLiterals", description = "Maximum number of literals.", required = false)
	private int maximumNumberOfLiterals = 3;

	@Parameter(names = "-maxVariables", description = "Maximum number of variables.", required = false)
	private int maximumNumberOfVariables = 3;

	@Parameter(names = "-includeConstants", description = "Whether clauses with constants should be included or not.", required = false)
	private boolean includeConstants = false;

	@Parameter(names = "-domainFile", description = "Path to the domain input MLN file.", required = true)
	private String domainFile = "";

	@Parameter(names = "-templateFile", description = "Path to the template output CSV file.", required = true)
	private String templateFile = "";

	@Parameter(names = "-formulaFile", description = "Path to the formula output CSV file.", required = true)
	private String formulaFile = "";

	@Parameter(names = "-formulaDirectory", description = "Path to the formula directory.", required = true)
	private String formulaDirectory = "";

	public int getMinimumNumberOfLiterals() {
		return this.minimumNumberOfLiterals;
	}

	public int getMaximumNumberOfLiterals() {
		return this.maximumNumberOfLiterals;
	}

	public int getMaximumNumberOfVariables() {
		return this.maximumNumberOfVariables;
	}

	public boolean includeConstants() {
		return this.includeConstants;
	}

	public File getDomainFile() {
		return new File(this.domainFile);
	}

	public File getTemplateFile() {
		return new File(this.templateFile);
	}

	public File getFormulaFile() {
		return new File(this.formulaFile);
	}

	public File getFormulaDirectory() {
		return new File(this.formulaDirectory);
	}

}