package ranker.template;

import java.util.List;

import ranker.formula.Formula;
import ranker.formula.FormulaComparator;
import util.SortedList;

/**
 * @author Jan Van Haaren
 * @version 1.0
 */
public class Template {

	private final int templateId;

	private final List<Formula> formulas;

	private final boolean containsConstants;

	private final String representation;

	private double multiplier;

	public Template(int templateId, boolean containsConstants, String representation) {
		this.templateId = templateId;
		this.formulas = new SortedList<Formula>(FormulaComparator.getInstance());
		this.containsConstants = containsConstants;
		this.representation = representation;
		this.multiplier = 1;
	}

	public Template(Template template) {
		this.templateId = template.getTemplateId();
		this.formulas = new SortedList<Formula>(FormulaComparator.getInstance());
		for (Formula formula : template.getFormulas()) {
			this.addFormula(new Formula(formula, this));
		}
		this.containsConstants = template.containsConstants();
		this.representation = template.getRepresentation();
		this.multiplier = template.getMultiplier();
	}

	public int getTemplateId() {
		return this.templateId;
	}

	public List<Formula> getFormulas() {
		return this.formulas;
	}

	public void addFormula(Formula formula) {
		this.getFormulas().add(formula);
	}

	public boolean containsConstants() {
		return this.containsConstants;
	}

	public String getRepresentation() {
		return this.representation;
	}

	public double getScore() {
		double total = 0;
		for (Formula formula : this.getFormulas()) {
			total += formula.getScore();
		}
		return total / this.getFormulas().size();
	}

	public double getMultiplier() {
		return this.multiplier;
	}

	public void setMultiplier(double multiplier) {
		this.multiplier = multiplier;
	}

}
