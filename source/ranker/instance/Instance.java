package ranker.instance;

import ranker.formula.Formula;

/**
 * @author Jan Van Haaren
 * @version 1.0
 */
public class Instance {

	private final Formula formula;

	private double score;

	private final String instance;

	public Instance(Formula formula, double score, String instance) {
		this.formula = formula;
		this.score = score;
		this.instance = instance;
	}

	public Instance(Instance instance, Formula formula) {
		this.formula = formula;
		this.score = instance.getScore();
		this.instance = instance.getInstance();
	}

	public Formula getFormula() {
		return this.formula;
	}

	public double getScore() {
		return this.score * this.getFormula().getTemplate().getMultiplier();
	}

	public void setScore(double score) {
		this.score = score;
	}

	public String getInstance() {
		return this.instance;
	}

	public boolean isDefinite() {
		String[] literals = this.getInstance().split(" v ");
		int numberOfLiterals = literals.length;
		int numberOfNegations = 0;
		for (String literal : literals) {
			if (literal.startsWith("!")) {
				numberOfNegations++;
			}
		}
		if (numberOfLiterals == numberOfNegations + 1) {
			return true;
		}
		else {
			return false;
		}
	}

}
