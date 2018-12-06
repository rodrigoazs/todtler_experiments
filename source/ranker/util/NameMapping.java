package ranker.util;

import java.io.File;
import java.io.IOException;
import java.util.ArrayList;
import java.util.HashMap;
import java.util.List;
import java.util.Map;

import org.apache.commons.io.FileUtils;

/**
 * @author Jan Van Haaren
 * @version 1.0
 */
public class NameMapping {

	private final Map<String, String> templateMapping;

	private final Map<String, String> formulaMapping;

	public NameMapping(final File templateInputFile, final File formulaInputFile) {
		this.templateMapping = readMappingFromFile(templateInputFile);
		this.formulaMapping = readMappingFromFile(formulaInputFile);
	}

	public String getTemplateMapping(int templateId) {
		return this.getTemplateMapping().get(String.valueOf(templateId));
	}

	private Map<String, String> getTemplateMapping() {
		return this.templateMapping;
	}

	public String getFormulaMapping(int formulaId, boolean isConstantVersion) {
		String suffix = isConstantVersion ? "-constant" : "";
		return this.getFormulaMapping().get(String.valueOf(formulaId + suffix));
	}

	public Map<String, String> getFormulaMapping() {
		return this.formulaMapping;
	}

	private static Map<String, String> readMappingFromFile(File file) {
		Map<String, String> result = new HashMap<String, String>();
		for (String line : readFile(file)) {
			result.put(line.split("\t")[0], line.split("\t")[1]);
		}
		return result;
	}

	private static List<String> readFile(File file) {
		List<String> content = new ArrayList<String>();
		try {
			content = FileUtils.readLines(file);
		}
		catch (IOException e) {
			e.printStackTrace();
		}
		return content;
	}

}
