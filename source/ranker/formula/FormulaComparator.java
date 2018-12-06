package ranker.formula;

import java.util.Comparator;

/**
 * @author Jan Van Haaren
 * @version 1.0
 */
public class FormulaComparator implements Comparator<Formula> {

	private static FormulaComparator INSTANCE;

	private FormulaComparator() {
		// NOP
	}

	public static FormulaComparator getInstance() {
		if (INSTANCE == null) {
			INSTANCE = new FormulaComparator();
		}
		return INSTANCE;
	}

	@Override
	public int compare(Formula formula1, Formula formula2) {
		if (formula1.getScore() < formula2.getScore()) {
			return 1;
		}
		else if (formula1.getScore() > formula2.getScore()) {
			return -1;
		}
		else {
			return 0;
		}
	}

}
