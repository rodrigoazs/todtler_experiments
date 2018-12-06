package ranker.formula;

import java.util.List;

import ranker.instance.Instance;
import ranker.instance.InstanceComparator;
import ranker.template.Template;
import util.SortedList;

/**
 * @author Jan Van Haaren
 * @version 1.0
 */
public class Formula {

	private final Template template;

	private final List<Instance> instances;

	private final int formulaId;

	private final String representation;

	public Formula(Template template, int formulaId, String representation) {
		this.template = template;
		this.instances = new SortedList<Instance>(InstanceComparator.getInstance());
		this.formulaId = formulaId;
		this.representation = representation;
	}

	public Formula(Formula formula, Template template) {
		this.template = template;
		this.instances = new SortedList<Instance>(InstanceComparator.getInstance());
		for (Instance instance : formula.getInstances()) {
			this.addInstance(new Instance(instance, this));
		}
		this.formulaId = formula.getFormulaId();
		this.representation = formula.getRepresentation();
	}

	public Template getTemplate() {
		return this.template;
	}

	public List<Instance> getInstances() {
		return this.instances;
	}

	public void addInstance(Instance instance) {
		this.getInstances().add(instance);
	}

	public int getFormulaId() {
		return this.formulaId;
	}

	public String getRepresentation() {
		return this.representation;
	}

	public double getScore() {
		double total = 0;
		for (Instance instance : this.getInstances()) {
			total += instance.getScore();
		}
		return total / this.getInstances().size();
	}

}
