package ranker.settings;

import java.io.File;

import com.beust.jcommander.Parameter;

/**
 * @author Jan Van Haaren
 * @version 1.0
 * 
 *          This class contains the settings of the ranker.
 */
public class Settings {

	@Parameter(names = "-domainName", description = "Domain name.", required = true)
	private String domainName = "";

	@Parameter(names = "-directory", description = "Path to the clause input directory.", required = true)
	private String directory = "";

	@Parameter(names = "-templateInputFile", description = "Path to the template input CSV file.", required = true)
	private String templateInputFile = "";

	@Parameter(names = "-formulaInputFile", description = "Path to the formula input CSV file.", required = true)
	private String formulaInputFile = "";

	@Parameter(names = "-templateOutputFile", description = "Path to the template output CSV file.", required = true)
	private String templateOutputFile = "";

	@Parameter(names = "-formulaOutputFile", description = "Path to the formula output CSV file.", required = true)
	private String formulaOutputFile = "";

	public String getDomainName() {
		return this.domainName;
	}

	public File getDirectory() {
		return new File(this.directory);
	}

	public File getTemplateInputFile() {
		return new File(this.templateInputFile);
	}

	public File getFormulaInputFile() {
		return new File(this.formulaInputFile);
	}

	public File getTemplateOutputFile() {
		return new File(this.templateOutputFile);
	}

	public File getFormulaOutputFile() {
		return new File(this.formulaOutputFile);
	}

}