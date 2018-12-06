package learner.settings;

import learner.LearnStrategy;

import com.beust.jcommander.IParameterValidator;
import com.beust.jcommander.ParameterException;

/**
 * @author Jan Van Haaren
 * @version 1.0
 */
class LearnStrategyValidator implements IParameterValidator {

	@Override
	public void validate(String name, String value) throws ParameterException {
		boolean valid = false;
		for (LearnStrategy learnStrategy : LearnStrategy.values()) {
			if (learnStrategy.name().equalsIgnoreCase(value.replaceAll("-", "_"))) {
				valid = true;
			}
		}
		if (!valid) {
			throw new ParameterException("Parameter " + name + " is not valid.");
		}
	}

}