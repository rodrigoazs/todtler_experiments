package ranker;

import java.util.ArrayList;
import java.util.HashMap;
import java.util.List;
import java.util.Map;

import ranker.formula.Formula;
import ranker.formula.FormulaComparator;
import ranker.instance.Instance;
import ranker.instance.InstanceComparator;
import ranker.template.Template;
import ranker.template.TemplateComparator;
import ranker.template.TemplateType;
import util.SortedList;

/**
 * @author Jan Van Haaren
 * @version 1.0
 */
public class Ranking {

	private final List<Template> templates;

	Ranking() {
		this.templates = new SortedList<Template>(TemplateComparator.getInstance());
	}

	public List<Template> getTemplates(TemplateType templateType) {
		List<Template> result = new SortedList<Template>(TemplateComparator.getInstance());
		for (Template template : this.getTemplatesList()) {
			if (templateType.isValidTemplate(template)) {
				result.add(template);
			}
		}
		return result;
	}

	public List<Formula> getFormulas(TemplateType templateType) {
		List<Formula> result = new SortedList<Formula>(FormulaComparator.getInstance());
		for (Template template : this.getTemplates(templateType)) {
			result.addAll(template.getFormulas());
		}
		return result;
	}

	private List<Template> getTemplatesList() {
		return this.templates;
	}

	public Map<Integer, Template> getTemplatesMap() {
		Map<Integer, Template> result = new HashMap<Integer, Template>();
		for (Template template : this.getTemplatesList()) {
			result.put(template.getTemplateId(), template);
		}
		return result;
	}

	void addTemplates(List<Template> templates) {
		this.getTemplatesList().addAll(templates);
	}

	public List<Instance> getInstances() {
		List<Instance> result = new SortedList<Instance>(InstanceComparator.getInstance());
		for (Template template : this.getTemplatesList()) {
			for (Formula formula : template.getFormulas()) {
				result.addAll(formula.getInstances());
			}
		}
		return result;
	}

	void normalize() {
		double maximumScore = this.getMaximumScore();
		double minimumScore = this.getMinimumScore();
		for (Instance instance : this.getInstances()) {
			instance.setScore(this.normalize(instance.getScore(), minimumScore, maximumScore));
		}
	}

	private double normalize(double score, double minimumScore, double maximumScore) {
		return (score - minimumScore) / (maximumScore - minimumScore);
	}

	private double getMaximumScore() {
		double result = -10000;
		for (Instance instance : this.getInstances()) {
			result = Math.max(result, instance.getScore());
		}
		return result;
	}

	private double getMinimumScore() {
		double result = 10000;
		for (Instance instance : this.getInstances()) {
			result = Math.min(result, instance.getScore());
		}
		return result;
	}

	public Ranking merge(Ranking otherRanking, MergeOperator operator) {

		Map<Integer, Template> map = otherRanking.getTemplatesMap();

		List<Template> templates = new ArrayList<Template>();
		for (Template template : this.getTemplatesList()) {
			Template newTemplate = new Template(template);
			templates.add(newTemplate);
			Template otherTemplate = map.get(newTemplate.getTemplateId());
			if (otherTemplate != null) {
				newTemplate.setMultiplier(otherTemplate.getScore());
			}
		}

		Ranking ranking = new Ranking();
		ranking.addTemplates(templates);
		return ranking;
	}
}
