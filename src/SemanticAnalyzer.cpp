#include "SemanticAnalyzer.h"
#include "assert.h"

namespace CppInterp {

	TypeInfo::TypeInfo() : m_kind(UNKNOWN) {}

	TypeInfo::TypeInfo(Kind k, const std::string& n) : m_kind(k), m_name(n) {}

	bool TypeInfo::IsSame(const TypeInfo& other) const {
		if (m_kind != other.m_kind) return false;
		switch (m_kind) {
		case BUILTIN:
		case STRUCT:
			return m_name == other.m_name;
		case ARRAY:
			return m_elementType->IsSame(*other.m_elementType);
		case FUNCTION:
			if (m_paramTypes.size() != other.m_paramTypes.size()) return false;
			for (size_t i = 0; i < m_paramTypes.size(); i++)
				if (!m_paramTypes[i]->IsSame(*other.m_paramTypes[i])) return false;
			return m_returnType->IsSame(*other.m_returnType);
		default: return false;
		}
	}

	TypeRegistry::TypeRegistry() {
		InitBuiltins();
	}

	std::optional<TypeInfo*> TypeRegistry::Find(const std::string& name) const {
		if (auto it = m_builtinTypes.find(name); it != m_builtinTypes.end())
			return { it->second };
		if (auto it = m_structTypes.find(name); it != m_structTypes.end())
			return { it->second };
		return std::nullopt;
	}

	TypeInfo* TypeRegistry::RegisterStruct(const std::string& name) {
		if (auto it = m_structTypes.find(name); it != m_structTypes.end())
			return it->second;
		auto* t = new TypeInfo{ TypeInfo::STRUCT, name };
		m_structTypes[name] = t;
		return t;
	}

	TypeInfo* TypeRegistry::GetOrCreateArray(TypeInfo* elem) {
		std::string key = elem->m_name + "[]";
		if (auto it = m_arrayTypes.find(key); it != m_arrayTypes.end())
			return it->second;
		auto* t = new TypeInfo{ TypeInfo::ARRAY, key };
		t->m_elementType = elem;
		m_arrayTypes[key] = t;
		return t;
	}

	TypeInfo* TypeRegistry::GetOrCreateFunction(const std::vector<TypeInfo*>& params, TypeInfo* ret) {
		std::string key = ret->m_name + "(";
		for (auto* p : params) key += p->m_name + ",";
		key += ")";
		if (auto it = m_functionTypes.find(key); it != m_functionTypes.end())
			return it->second;
		auto* t = new TypeInfo{ TypeInfo::FUNCTION, key };
		t->m_returnType = ret;
		t->m_paramTypes = params;
		m_functionTypes[key] = t;
		return t;
	}

	void TypeRegistry::InitBuiltins() {
		m_builtinTypes.emplace("int", new TypeInfo{ TypeInfo::BUILTIN,"int" });
		m_builtinTypes.emplace("double", new TypeInfo{ TypeInfo::BUILTIN,"double" });
		m_builtinTypes.emplace("char", new TypeInfo{ TypeInfo::BUILTIN,"char" });
		m_builtinTypes.emplace("string", new TypeInfo{ TypeInfo::BUILTIN,"string" });
		m_builtinTypes.emplace("bool", new TypeInfo{ TypeInfo::BUILTIN,"bool" });
		m_builtinTypes.emplace("void", new TypeInfo{ TypeInfo::BUILTIN,"void" });
	}

	TypeRegistry::~TypeRegistry() {
		for (auto& pair : m_builtinTypes)
			delete pair.second;
		for (auto& pair : m_structTypes)
			delete pair.second;
		for (auto& pair : m_arrayTypes)
			delete pair.second;
		for (auto& pair : m_functionTypes)
			delete pair.second;
	}

	void AstConverter::ClearNodes() {
		for (auto node : m_nodes)
			delete node;
		m_root = nullptr;
	}

	AstConverter::~AstConverter() {
		ClearNodes();
	}

	SemanticAstNode* AstConverter::ConvertToSemanticAst(AstNode* root) {
		if (!root)
			return nullptr;
		ClearNodes();
		m_root = ConvertNode(root);
		return m_root;
	}

	SemanticAstNode* AstConverter::ConvertNode(AstNode* node) {
		switch (node->m_type) {
		case NodeType::PROGRAM:
			return ConvertProgram(node);
		case NodeType::IMPORT_STMT:
			return ConvertImportStmt(node);
		case NodeType::FUNCTION_DECL:
			return ConvertFunctionDecl(node);
		case NodeType::PARAMETER:
			return ConvertParameter(node);
		case NodeType::COMPOUND_STMT:
			return ConvertCompoundStmt(node);
		case NodeType::EMPTY_STMT:
			return ConvertEmptyStmt(node);
		case NodeType::VAR_DECL:
			return ConvertVariableDecl(node);
		case NodeType::DECLARATOR_LIST:
			return ConvertDeclaratorList(node);
		case NodeType::DECLARATOR:
			return ConvertDeclarator(node);
		case NodeType::IF_STMT:
			return ConvertIfStmt(node);
		case NodeType::CASE_CLAUSE:
			return ConvertCaseClause(node);
		case NodeType::WHILE_STMT:
			return ConvertWhileStmt(node);
		case NodeType::DEFAULT_CLAUSE:
			return ConvertDefaultClause(node);
		case NodeType::FOR_STMT:
			return ConvertForStmt(node);
		case NodeType::RETURN_STMT:
			return ConvertReturnStmt(node);
		case NodeType::BREAK_STMT:
			return ConvertBreakStmt(node);
		case NodeType::CONTINUE_STMT:
			return ConvertContinueStmt(node);
		case NodeType::STRUCT_DECL:
			return ConvertStructDecl(node);
		case NodeType::STRUCT_DECLARATOR_LIST:
			return ConvertStructDeclaratorList(node);
		case NodeType::STRUCT_MEMBER_DECL:
			return ConvertMemberDecl(node);
		case NodeType::EXPRESSION:
			return ConvertExpression(node);
		case NodeType::ASSIGN_EXPR:
			return ConvertAssignExpr(node);
		case NodeType::CONDITIONAL_EXPR:
			return ConvertConditionExpr(node);
		case NodeType::BINARY_EXPR:
			return ConvertBinaryExpr(node);
		case NodeType::UNARY_EXPR:
			return ConvertUnaryExpr(node);
		case NodeType::POSTFIX_EXPR:
			return ConvertPostfixExpr(node);
		case NodeType::CALL_EXPR:
			return ConvertCallExpr(node);
		case NodeType::INDEX_EXPR:
			return ConvertIndexExpr(node);
		case NodeType::MEMBER_EXPR:
			return ConvertMemberExpr(node);
		case NodeType::GROUP_EXPR:
			return ConvertGroupExpr(node);
		case NodeType::INITIALIZER:
			return ConvertInitializer(node);
		case NodeType::ARGUMENT_LIST:
			return ConvertArgumentList(node);
		case NodeType::FUNCTION_LITERAL:
			return ConvertFunctionLiteral(node);
		case NodeType::PRIMARY:
			return ConvertPrimary(node);
		case NodeType::LITERAL:
			return ConvertLiteral(node);
		case NodeType::PARAMETER_TYPE_LIST:
			return ConvertParameterTypeList(node);
		default:
			throw std::runtime_error("Unknown AST node type during conversion");
		}
	}


	SemanticAstNode* AstConverter::ConvertProgram(AstNode* node) {
		auto* cur = new ProgramNode();
		for (auto child : node->m_children) {
			auto decl = ConvertNode(child);
			cur->m_declarations.push_back(decl);
		}
		m_nodes.push_back(cur);
		return cur;
	}

	SemanticAstNode* AstConverter::ConvertImportStmt(AstNode* node) {
		assert(node->m_children.size() == 1);
		auto* cur = new ImportStmt();
		cur->m_moduleName = node->m_token.m_content;
		m_nodes.push_back(cur);
		return cur;
	}

	SemanticAstNode* AstConverter::ConvertFunctionDecl(AstNode* node) {
		assert(node->m_children.size() == 4);
		auto* cur = new FunctionDecl();
		cur->m_returnType = ConvertType(node->m_children[0]);
		cur->m_name = node->m_children[1]->m_token.m_content;
		cur->m_parameters = ConvertParameterList(node->m_children[2]);
		cur->m_body = dynamic_cast<CompoundStmt*>(ConvertCompoundStmt(node->m_children[3]));
		m_nodes.push_back(cur);
		return cur;
	}

	SemanticAstNode* AstConverter::ConvertCompoundStmt(AstNode* node) {
		auto* cur = new FunctionDecl();
		return cur;
	}
	SemanticAstNode* AstConverter::ConvertEmptyStmt(AstNode* node) {
		auto* cur = new FunctionDecl();
		return cur;
	}
	Declarator* AstConverter::ConvertDeclarator(AstNode* node) {
		auto* cur = new Declarator();
		int childNums = node->m_children.size();
		assert(childNums >= 1);
		cur->m_name = node->m_children[0]->m_token.m_content;
		if (childNums > 1) {
			cur->m_arrayDimensions = ConvertArraySize(node->m_children[1]);
		}
		if (childNums > 2) {
			cur->m_initializer = dynamic_cast<Expression*>(ConvertNode(node->m_children[2]));
		}
		return cur;
	}
	SemanticAstNode* AstConverter::ConvertIfStmt(AstNode* node) {
		auto* cur = new FunctionDecl();
		return cur;
	}
	SemanticAstNode* AstConverter::ConvertCaseClause(AstNode* node) {
		auto* cur = new FunctionDecl();
		return cur;
	}
	SemanticAstNode* AstConverter::ConvertWhileStmt(AstNode* node) {
		auto* cur = new FunctionDecl();
		return cur;
	}
	SemanticAstNode* AstConverter::ConvertDefaultClause(AstNode* node) {
		auto* cur = new FunctionDecl();
		return cur;
	}
	SemanticAstNode* AstConverter::ConvertForStmt(AstNode* node) {
		auto* cur = new FunctionDecl();
		return cur;
	}
	SemanticAstNode* AstConverter::ConvertReturnStmt(AstNode* node) {
		auto* cur = new FunctionDecl();
		return cur;
	}
	SemanticAstNode* AstConverter::ConvertBreakStmt(AstNode* node) {
		auto* cur = new FunctionDecl();
		return cur;
	}
	SemanticAstNode* AstConverter::ConvertContinueStmt(AstNode* node) {
		auto* cur = new FunctionDecl();
		return cur;
	}
	SemanticAstNode* AstConverter::ConvertStructDecl(AstNode* node) {
		auto* cur = new FunctionDecl();
		return cur;
	}
	SemanticAstNode* AstConverter::ConvertStructDeclaratorList(AstNode* node) {
		auto* cur = new FunctionDecl();
		return cur;
	}
	SemanticAstNode* AstConverter::ConvertMemberDecl(AstNode* node) {
		auto* cur = new FunctionDecl();
		return cur;
	}
	SemanticAstNode* AstConverter::ConvertExpression(AstNode* node) {
		auto* cur = new FunctionDecl();
		return cur;
	}
	SemanticAstNode* AstConverter::ConvertAssignExpr(AstNode* node) {
		auto* cur = new FunctionDecl();
		return cur;
	}
	SemanticAstNode* AstConverter::ConvertConditionExpr(AstNode* node) {
		auto* cur = new FunctionDecl();
		return cur;
	}
	SemanticAstNode* AstConverter::ConvertBinaryExpr(AstNode* node) {
		auto* cur = new FunctionDecl();
		return cur;
	}
	SemanticAstNode* AstConverter::ConvertUnaryExpr(AstNode* node) {
		auto* cur = new FunctionDecl();
		return cur;
	}
	SemanticAstNode* AstConverter::ConvertPostfixExpr(AstNode* node) {
		auto* cur = new FunctionDecl();
		return cur;
	}
	SemanticAstNode* AstConverter::ConvertCallExpr(AstNode* node) {
		auto* cur = new FunctionDecl();
		return cur;
	}
	SemanticAstNode* AstConverter::ConvertIndexExpr(AstNode* node) {
		auto* cur = new FunctionDecl();
		return cur;
	}
	SemanticAstNode* AstConverter::ConvertMemberExpr(AstNode* node) {
		auto* cur = new FunctionDecl();
		return cur;
	}
	SemanticAstNode* AstConverter::ConvertGroupExpr(AstNode* node) {
		auto* cur = new FunctionDecl();
		return cur;
	}
	SemanticAstNode* AstConverter::ConvertDesignedInitializer(AstNode* node) {
		auto* cur = new FunctionDecl();
		return cur;
	}
	SemanticAstNode* AstConverter::ConvertArgumentList(AstNode* node) {
		auto* cur = new FunctionDecl();
		return cur;
	}
	SemanticAstNode* AstConverter::ConvertFunctionLiteral(AstNode* node) {
		auto* cur = new FunctionDecl();
		return cur;
	}
	SemanticAstNode* AstConverter::ConvertPrimary(AstNode* node) {
		auto* cur = new FunctionDecl();
		return cur;
	}
	SemanticAstNode* AstConverter::ConvertLiteral(AstNode* node) {
		auto* cur = new FunctionDecl();
		return cur;
	}
	SemanticAstNode* AstConverter::ConvertParameterTypeList(AstNode* node) {
		auto* cur = new FunctionDecl();
		return cur;
	}

	std::vector<VariableDecl*> AstConverter::ConvertParameterList(AstNode* node) {
		std::vector<VariableDecl*> params;
		for (auto paramNode : node->m_children[2]->m_children) {
			params.push_back(ConvertParameter(paramNode));
		}
		return params;
	}

	VariableDecl* AstConverter::ConvertParameter(AstNode* node) {
		auto* cur = new VariableDecl();
		assert(node->m_children.size() == 2);
		AstNode* typeNode = node->m_children[0];
		cur->m_isConst = node->m_token.m_type == TokenType::CONST ? true : false;
		cur->m_type = ConvertType(typeNode);
		if (node->m_children.size() > 1 && node->m_children[1]->m_type == NodeType::ARRAY_SIZE) {
			cur->m_type = m_typeRegistry.GetOrCreateArray(cur->m_type);
		}
		cur->m_declarators.push_back(ConvertDeclarator(node->m_children[1]));
		return cur;
	}

	SemanticAstNode* AstConverter::ConvertInitializer(AstNode* node) {
		auto* cur = new InitializerExpr();
		for (auto* child : node->m_children) {

		}
		return cur;
	}

	TypeInfo* AstConverter::ConvertType(AstNode* node) {
		const Token& token = node->m_token;
		switch (node->m_type)
		{
		case NodeType::BUILTIN_TYPE:
		{
			if (auto optType = m_typeRegistry.Find(token.m_content); optType.has_value()) {
				return optType.value();
			}
			else {
				throw SemanticException("Unknown builtin type: " + token.m_content, token.m_column, token.m_line);
			}
		}
		case NodeType::IDENTIFIER:
		{
			if (auto optType = m_typeRegistry.Find(token.m_content); optType.has_value()) {
				return optType.value();
			}
			else {
				return m_typeRegistry.RegisterStruct(token.m_content);
			}
		}
		case NodeType::FUNCTION_TYPE:
		{
			assert(node->m_children.size() == 2);
			std::vector<TypeInfo*> paramTypes;
			AstNode* paramListNode = node->m_children[0];
			if (paramListNode->m_type == TokenType::IDENTIFIER)
			{
				// single parameter
				paramTypes.push_back(ConvertType(paramListNode));
			}
			else {
				// parameter list
				for (auto* paramNode : paramListNode->m_children) {
					TypeInfo* paramType = ConvertType(paramNode);
					paramTypes.push_back(paramType);
				}
			}
			TypeInfo* returnType = ConvertType(node->m_children[1]);
			return m_typeRegistry.GetOrCreateFunction(paramTypes, returnType);
		}
		default:
			throw SemanticException("Convert error: unexpected astnode type " +
				NodeTypeToString(node->m_type), token.m_column, token.m_line);
		}
	}

	std::vector<Expression*> AstConverter::ConvertArraySize(AstNode* node) {
		std::vector<Expression*> dimens;
		for (auto* dim : node->m_children) {
			dimens.push_back(dynamic_cast<Expression*>(ConvertNode(dim)));
		}
		return dimens;
	}

	SemanticAnalyzer::SemanticAnalyzer(AstNode* astRoot) :m_astRoot(astRoot) {

	}

	void SemanticAnalyzer::SemanticAnalyze() {

	}
}