package generator.template;

import java.util.HashMap;
import java.util.List;
import java.util.Map;

import org.jgraph.graph.DefaultEdge;
import org.jgrapht.UndirectedGraph;
import org.jgrapht.alg.ConnectivityInspector;
import org.jgrapht.graph.SimpleGraph;

import util.SortedList;

/**
 * @author Jan Van Haaren
 * @version 1.0
 * 
 *          This class represents a second-order logic formula.
 */
public class Template {

	/**
	 * A list containing the literals appearing in this template.
	 */
	private final List<TemplateLiteral> literals;

	/**
	 * Constructs a new template.
	 */
	public Template() {
		this.literals = new SortedList<TemplateLiteral>(TemplateLiteralComparator.getInstance());
	}

	/**
	 * Constructs a template by copying the given template.
	 * 
	 * @param template
	 *            The given template.
	 */
	public Template(Template template) {
		this.literals = new SortedList<TemplateLiteral>(TemplateLiteralComparator.getInstance());
		for (TemplateLiteral literal : template.getLiterals()) {
			this.addLiteral(new TemplateLiteral(literal));
		}
	}

	/**
	 * Returns a list containing the literals appearing in this template.
	 * 
	 * @return The literals appearing in this template.
	 */
	public List<TemplateLiteral> getLiterals() {
		return this.literals;
	}

	/**
	 * Adds the given literal to this template.
	 * 
	 * @param literal
	 *            The given literal.
	 */
	public void addLiteral(TemplateLiteral literal) {
		this.getLiterals().add(new TemplateLiteral(literal));
	}

	/**
	 * Returns the number of literals appearing in this template.
	 * 
	 * @return The number of literals.
	 */
	public int getLength() {
		return this.getLiterals().size();
	}

	/**
	 * Normalizes the template.
	 */
	public void normalize() {
		Map<Integer, Integer> argumentCache = new HashMap<Integer, Integer>();
		Map<Integer, Integer> literalCache = new HashMap<Integer, Integer>();
		for (TemplateLiteral literal : this.getLiterals()) {
			literal.setLiteralNumber(getValueFromCache(literal.getLiteralNumber(), literalCache));
			literal.setArgument1Number(getValueFromCache(literal.getArgument1Number(), argumentCache));
			literal.setArgument2Number(getValueFromCache(literal.getArgument2Number(), argumentCache));
		}
	}

	/**
	 * Verifies if the literals appearing in this template are connected through variables.
	 * 
	 * @return Returns true if the template is connected, returns false otherwise.
	 */
	public boolean isConnected() {
		UndirectedGraph<String, DefaultEdge> graph = new SimpleGraph<String, DefaultEdge>(DefaultEdge.class);
		for (TemplateLiteral literal : this.getLiterals()) {
			graph.addVertex("v" + literal.hashCode());
		}
		for (TemplateLiteral literal1 : this.getLiterals()) {
			for (TemplateLiteral literal2 : this.getLiterals()) {
				if (!literal1.equals(literal2)) {
					if (literal1.getArgument1Number() == literal2.getArgument1Number() || literal1.getArgument1Number() == literal2.getArgument2Number() || literal2.getArgument1Number() == literal1.getArgument2Number()
							|| literal2.getArgument2Number() == literal1.getArgument2Number()) {
						graph.addEdge("v" + literal1.hashCode(), "v" + literal2.hashCode());
					}
				}
			}
		}
		return new ConnectivityInspector<String, DefaultEdge>(graph).isGraphConnected();
	}

	/**
	 * Verifies if any literal appearing in this template refers to itself.
	 * 
	 * @return Returns true if at least one literal refers to itself, returns false otherwise.
	 */
	public boolean refersToSelf() {
		for (TemplateLiteral literal : this.getLiterals()) {
			if (literal.getArgument1Number() == literal.getArgument2Number()) {
				return true;
			}
		}
		return false;
	}

	@Override
	public boolean equals(Object object) {
		if (Template.class.isAssignableFrom(object.getClass())) {
			Template otherTemplate = (Template) object;
			return this.hashCode() == otherTemplate.hashCode();
		}
		else {
			return false;
		}
	}

	@Override
	public int hashCode() {
		int result = 1;
		for (TemplateLiteral literal : this.getLiterals()) {
			result *= literal.hashCode();
		}
		return result;
	}

	@Override
	public String toString() {
		String representation = "";
		String prefix = "";
		for (TemplateLiteral literal : this.getLiterals()) {
			representation += prefix;
			representation += literal.toString();
			prefix = " v ";
		}
		return representation;
	}

	private static int getValueFromCache(int value, Map<Integer, Integer> cache) {
		if (!cache.containsKey(value)) {
			cache.put(value, cache.keySet().size() + 1);
		}
		return cache.get(value);
	}

}
