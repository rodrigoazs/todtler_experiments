package learner.settings;

import ranker.MergeOperator;

import com.beust.jcommander.IParameterValidator;
import com.beust.jcommander.ParameterException;

/**
 * @author Jan Van Haaren
 * @version 1.0
 */
class MergeOperatorValidator implements IParameterValidator {

	@Override
	public void validate(String name, String value) throws ParameterException {
		boolean valid = false;
		for (MergeOperator mergeOperator : MergeOperator.values()) {
			if (mergeOperator.name().equalsIgnoreCase(value)) {
				valid = true;
			}
		}
		if (!valid) {
			throw new ParameterException("Parameter " + name + " is not valid.");
		}
	}

}