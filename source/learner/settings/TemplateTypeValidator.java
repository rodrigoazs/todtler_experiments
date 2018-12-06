package learner.settings;

import ranker.template.TemplateType;

import com.beust.jcommander.IParameterValidator;
import com.beust.jcommander.ParameterException;

/**
 * @author Jan Van Haaren
 * @version 1.0
 */
class TemplateTypeValidator implements IParameterValidator {

	@Override
	public void validate(String name, String value) throws ParameterException {
		boolean valid = false;
		for (TemplateType templateType : TemplateType.values()) {
			if (templateType.name().equalsIgnoreCase(value)) {
				valid = true;
			}
		}
		if (!valid) {
			throw new ParameterException("Parameter " + name + " is not valid.");
		}
	}

}