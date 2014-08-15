#include <stdio.h>
#include "Evaluate.h"
#include "Token.h"
#include "TokenDebug.h"
#include <malloc.h>
#include "Stack.h"
#include "OperatorToken.h"
#include "NumberToken.h"
#include "Operator.h"
#include "ErrorCode.h"
#include "CException.h"


#define STACK_LENGTH 100
	
/*This function is perform try to push the operator into the operator stack
   while the operator stack is empty
   precedence of coming operator is higher then the precedence inside the operator stack, 
   then will push in and do for the next operator too
   else will start to perform operation

    input:
    ~operator , ~operatorStack , ~dataStack
    
    output:
    ~the returned number token and it is pushed into data stack, final return the data stack to the main function  */
void tryToPushOperatorAndEvaluate( Operator *opr, Stack *operatorStack,  Stack *dataStack ){
		
	Operator *ptrOpr;   
	ptrOpr = (Operator *)stackPeep(operatorStack);
  
  //Opr is the operator coming from the string expressoin
  //ptrOpr is the operator that coming from the operator stack
  
	if( (ptrOpr == NULL)  || (opr->info->precedence > ptrOpr->info->precedence)                                     ||
      (  (opr->info->precedence == ptrOpr->info->precedence) && ( opr->info->associativity == RIGHT_TO_LEFT ) )   ||
      (  (opr->info->precedence == ptrOpr->info->precedence) && ( opr->info->id == CLOSE_BRACKET ) )         ) 
	{	
		stackPush( operatorStack , opr );
	}
	else {
			while( ptrOpr != NULL)
			{     
				if  (opr->info->precedence <= ptrOpr->info->precedence || opr == NULL )
					{ 
						Operator *oprNew = stackPop( operatorStack);        
						oprNew->info->execute( dataStack , operatorStack );    
					}
					
				ptrOpr = (Operator *)stackPeep(operatorStack);
			}		
      stackPush( operatorStack , opr ); //while the operator stack is empty then push the opr into the last operator stack
	}
}

/*
This function is used to perform operation with the two stacks that already contain element
then start operation based on the elements of the stacks

input:
~ dataStack, ~operatorStack  */
void doOperatorStackRewinding ( Stack *dataStack , Stack *operatorStack ){

  Operator *ptrOpr;   // pointer to operator	
	ptrOpr = (Operator *)stackPeep(operatorStack);

  while( ptrOpr != NULL)
			{     
						Operator *oprNew = stackPop( operatorStack);        
						oprNew->info->execute( dataStack , operatorStack );    
				  	ptrOpr = (Operator *)stackPeep(operatorStack);
			}		
     
}


/* This function is used to verify the two stacks isn"t empty after the evaluation
  input:
  ~dataStack, ~operatorStack*/
void verifyAllStacksAreEmpty(Stack *dataStack, Stack *operatorStack) {

  if(stackPop( operatorStack) != NULL) {
    printf("Error: operatorStack is not empty\n");
    exit(EXIT_FAILURE);
  }
  if(stackPop( dataStack) != NULL) {
    printf("Error: dataStack is not empty\n");
    exit(EXIT_FAILURE);
  }
}

Token *convertToPrefixIfNotAlready(Operator *op) {
  if(op->info->affix == INFIX) {
    operatorTryConvertToPrefix(op);
    tokenDump((Token *)op);
  } 
  return (Token *)op;
}

void evaluatePrefixesAndNumber(Token *token, String *expression, Stack *dataStack, Stack *operatorStack) {
  if(token != NULL) {
    while(1) {
      if(token->type == NUMBER_TOKEN) {
      Number *num = (Number *)token;
      tokenDump(token);
      stackPush(dataStack, num);
      break;
      } else if(token->type == OPERATOR_TOKEN) {
        Operator *operator = (Operator *)token;
        tokenDump(token);
        token = convertToPrefixIfNotAlready(operator);
        stackPush(operatorStack, (Operator *)token);
      } else
        Throw(ERR_IDENTIFIER_NOT_SUPPORT);
      token = getToken(expression);
      if(token == NULL) 
        break;
    }
  }
  if(dataStack->size == 0)
    Throw(ERR_EXPECTING_NUMBER);
}


void evaluatePostfixesAndInfix(Token *token, String *expression, Stack *dataStack ,Stack *operatorStack ){

  Operator *bracketOpr;  // declare for bracket operator purpose

  if ( token->type == NUMBER_TOKEN)
      Throw (ERR_NOT_EXPECTING_NUMBER);    
  else {
      while (token != NULL){
          Operator *opr = (Operator*)token; 
          tryToPushOperatorAndEvaluate ( opr, operatorStack , dataStack  );
          token = getToken (expression);
          
          if(token == NULL)
          break;
      }
      bracketOpr = (Operator *)stackPeep(operatorStack);
      if ( bracketOpr->info->id == CLOSE_BRACKET )
        bracketOpr->info->execute( dataStack , operatorStack );      
    }
}

int evaluate(String *expression)
{
  int Result;
  Token *token;
  Stack *dataStack     = stackNew(STACK_LENGTH);
	Stack *operatorStack = stackNew(STACK_LENGTH);
  
  do{
    token = getToken(expression);

		if(token!=NULL) {
        
        evaluatePrefixesAndNumber(token, expression, dataStack, operatorStack);
        
        if(token == NULL)
          break;
  
        token = getToken(expression);
        evaluatePostfixesAndInfix(token, expression, operatorStack, dataStack);
        
    } 
      
  }while(token != NULL);
  
  
  doOperatorStackRewinding ( dataStack , operatorStack );
	Number *ans = (Number *)stackPop(dataStack);
	Result = ans->value;
	verifyAllStacksAreEmpty(dataStack, operatorStack);
  
	return Result;
}
